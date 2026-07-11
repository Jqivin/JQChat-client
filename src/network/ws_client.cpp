#include "ws_client.h"
#include "utils/logger.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QDateTime>
#include <QCryptographicHash>
#include <QUrl>
#include <QtEndian>
#include <QRandomGenerator>

static const quint8 WS_OPCODE_CONT = 0x0;
static const quint8 WS_OPCODE_TEXT = 0x1;
static const quint8 WS_OPCODE_BIN  = 0x2;
static const quint8 WS_OPCODE_CLOSE = 0x8;
static const quint8 WS_OPCODE_PING = 0x9;
static const quint8 WS_OPCODE_PONG = 0xA;

// 构造函数：初始化 Socket 和定时器
WsClient::WsClient(QObject *parent)
    : QObject(parent)
    , m_socket(new QSslSocket(this))
    , m_heartbeatTimer(new QTimer(this))
    , m_reconnectTimer(new QTimer(this))
{
    // 忽略自签名证书（生产环境应配置正确证书）
    m_socket->ignoreSslErrors();

    connect(m_socket, &QSslSocket::connected, this, &WsClient::onConnected);
    connect(m_socket, &QSslSocket::disconnected, this, &WsClient::onDisconnected);
    connect(m_socket, &QSslSocket::readyRead, this, &WsClient::onReadyRead);
    connect(m_heartbeatTimer, &QTimer::timeout, this, &WsClient::onHeartbeatTimeout);
    connect(m_reconnectTimer, &QTimer::timeout, this, &WsClient::onReconnectTimeout);
}

WsClient::~WsClient() {
    disconnect();
}

// 解析 WebSocket URL，携带 token 建立 TCP/TLS 连接
void WsClient::connectToServer(const QString &url, const QString &token) {
    Logger::instance().wsEvent("CONNECT", url);
    QUrl wsUrl(url);
    wsUrl.setQuery("token=" + token);
    m_host = wsUrl.host();
    m_useSsl = (wsUrl.scheme() == "wss");
    m_port = wsUrl.port(m_useSsl ? 443 : 80);
    m_path = wsUrl.path();
    if (m_path.isEmpty()) m_path = "/";
    m_path += "?token=" + token;
    m_token = token;
    m_reconnectInterval = 1000;
    m_reconnectAttempts = 0;
    m_handshakeDone = false;
    m_parseState = ParseHeader;

    if (m_useSsl) {
        m_socket->connectToHostEncrypted(m_host, m_port);
    } else {
        m_socket->connectToHost(m_host, m_port);
    }
}

// 断开连接：发送关闭帧后断开 Socket
void WsClient::disconnect() {
    m_heartbeatTimer->stop();
    m_reconnectTimer->stop();
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        QByteArray closePayload;
        closePayload.append(static_cast<char>(0x03E8 >> 8));
        closePayload.append(static_cast<char>(0x03E8 & 0xFF));
        sendFrame(closePayload, WS_OPCODE_CLOSE);
    }
    m_socket->disconnectFromHost();
}

bool WsClient::isConnected() const {
    return m_handshakeDone && m_socket->state() == QAbstractSocket::ConnectedState;
}

// Socket 连接成功：执行 WebSocket 握手
void WsClient::onConnected() {
    performHandshake();
}

// Socket 断开：停止心跳，尝试重连
void WsClient::onDisconnected() {
    Logger::instance().wsEvent("DISCONNECTED");
    m_handshakeDone = false;
    m_heartbeatTimer->stop();
    emit disconnected();
    tryReconnect();
}

// 发送 WebSocket HTTP 升级请求（Sec-WebSocket-Key 握手）
void WsClient::performHandshake() {
    QString key = "dGhlIHNhbXBsZSBub25jZQ==";
    QByteArray accept = QCryptographicHash::hash(
        (key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11").toUtf8(),
        QCryptographicHash::Sha1).toBase64();

    QString handshake = QString(
        "GET %1 HTTP/1.1\r\n"
        "Host: %2:%3\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Key: %4\r\n"
        "Sec-WebSocket-Version: 13\r\n"
        "\r\n")
        .arg(m_path, m_host, QString::number(m_port), key);

    m_socket->write(handshake.toUtf8());
    m_socket->flush();

    m_readBuffer.clear();
    m_handshakeDone = false;
}

// 读取 Socket 数据：先处理 HTTP 响应头，再持续解析 WebSocket 帧
void WsClient::onReadyRead() {
    QByteArray data = m_socket->readAll();
    m_readBuffer.append(data);

    if (!m_handshakeDone) {
        int idx = m_readBuffer.indexOf("\r\n\r\n");
        if (idx >= 0) {
            QString header = QString::fromUtf8(m_readBuffer.left(idx));
            if (header.contains(" 101 ")) {
                Logger::instance().wsEvent("HANDSHAKE_OK");
                m_handshakeDone = true;
                m_readBuffer.remove(0, idx + 4);
                m_reconnectAttempts = 0;
                m_heartbeatTimer->start(25000);
                emit connected();
            } else {
                Logger::instance().wsEvent("HANDSHAKE_FAIL", header.left(200));
                m_socket->disconnectFromHost();
                return;
            }
        }
        if (!m_handshakeDone) return;
    }

    // Parse frames
    while (m_readBuffer.size() > 0) {
        if (m_parseState == ParseHeader) {
            if (m_readBuffer.size() < 2) break;

            quint8 b0 = static_cast<quint8>(m_readBuffer[0]);
            quint8 b1 = static_cast<quint8>(m_readBuffer[1]);
            m_frameOpcode = b0 & 0x0F;
            m_frameMasked = (b1 & 0x80) != 0;
            m_framePayloadLen = b1 & 0x7F;
            int headerSize = 2;

            if (m_framePayloadLen == 126) {
                headerSize += 2;
                if (m_readBuffer.size() < headerSize) break;
                m_framePayloadLen = qFromBigEndian<quint16>(
                    reinterpret_cast<const uchar*>(m_readBuffer.constData() + 2));
            } else if (m_framePayloadLen == 127) {
                headerSize += 8;
                if (m_readBuffer.size() < headerSize) break;
                m_framePayloadLen = qFromBigEndian<quint64>(
                    reinterpret_cast<const uchar*>(m_readBuffer.constData() + 2));
            }

            if (m_frameMasked) {
                headerSize += 4;
                if (m_readBuffer.size() < headerSize) break;
                m_maskKey = m_readBuffer.mid(headerSize - 4, 4);
            }

            m_frameHeaderSize = headerSize;
            m_parseState = ParsePayload;
            m_framePayload.clear();
        }

        if (m_parseState == ParsePayload) {
            quint64 totalNeeded = m_frameHeaderSize + m_framePayloadLen;
            if (static_cast<quint64>(m_readBuffer.size()) < totalNeeded) break;

            QByteArray payload = m_readBuffer.mid(m_frameHeaderSize, m_framePayloadLen);

            if (m_frameMasked) {
                for (quint64 i = 0; i < m_framePayloadLen; ++i) {
                    payload[i] = payload[i] ^ m_maskKey[i % 4];
                }
            }

            m_readBuffer.remove(0, totalNeeded);
            m_parseState = ParseHeader;

            processFrame(payload, m_frameOpcode);
        }
    }
}

// 处理数据帧：根据 opcode 分发消息/心跳/在线状态等
void WsClient::processFrame(const QByteArray &data, quint8 opcode) {
    if (opcode == WS_OPCODE_TEXT) {
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
        if (!doc.isObject()) {
            Logger::instance().wsEvent("PARSE_ERR",
                QString("opcode=%1 len=%2 err=%3 data=%4")
                    .arg(opcode).arg(data.size()).arg(parseError.errorString())
                    .arg(QString::fromUtf8(data.left(100))));
            return;
        }

        QJsonObject obj = doc.object();
        QString type = obj["type"].toString();

        // heartbeat 不打印，其他消息打印
        if (type != "heartbeat") {
            Logger::instance().wsRecv(QString::fromUtf8(doc.toJson(QJsonDocument::Compact)));
        }

        QJsonObject msgData = obj["data"].toObject();

        if (type == "heartbeat") {
            // ignore
        } else if (type == "message") {
            MessageData msg;
            msg.msg_id = msgData["msg_id"].toString();
            msg.from_uid = msgData["from_uid"].toVariant().toULongLong();
            msg.to_uid = msgData["to_uid"].toVariant().toULongLong();
            msg.msg_type = msgData["msg_type"].toInt();
            msg.content = msgData["content"].toString();
            msg.file_url = msgData["file_url"].toString();
            msg.created_at = QDateTime::fromString(msgData["created_at"].toString(), Qt::ISODate);
            emit messageReceived(msg);
        } else if (type == "online_change") {
            quint64 uid = msgData["uid"].toVariant().toULongLong();
            bool online = msgData["online"].toBool();
            emit friendOnlineChanged(uid, online);
        } else if (type == "typing") {
            quint64 fromUid = msgData["to_uid"].toVariant().toULongLong();
            emit typingReceived(fromUid);
        } else if (type == "friend_request") {
            quint64 fromUid = msgData["from_uid"].toVariant().toULongLong();
            QString fromName = msgData["from_name"].toString();
            QString remark = msgData["remark"].toString();
            emit friendRequestReceived(fromUid, fromName, remark);
        }
    } else if (opcode == WS_OPCODE_PING) {
        sendPong();
    } else if (opcode == WS_OPCODE_CLOSE) {
        m_socket->disconnectFromHost();
    }
}

// 编码并发送 WebSocket 数据帧
void WsClient::sendFrame(const QByteArray &payload, quint8 opcode) {
    if (m_socket->state() != QAbstractSocket::ConnectedState) return;
    QByteArray frame = encodeFrame(payload, opcode);
    m_socket->write(frame);
    m_socket->flush();
}

// 按照 WebSocket 协议编码帧（FIN + opcode + Mask + Payload）
QByteArray WsClient::encodeFrame(const QByteArray &payload, quint8 opcode) {
    QByteArray frame;
    quint64 len = payload.size();

    // FIN + opcode
    frame.append(static_cast<char>(0x80 | opcode));

    // Masked + payload length
    if (len < 126) {
        frame.append(static_cast<char>(0x80 | len));
    } else if (len < 65536) {
        frame.append(static_cast<char>(0x80 | 126));
        frame.append(static_cast<char>((len >> 8) & 0xFF));
        frame.append(static_cast<char>(len & 0xFF));
    } else {
        frame.append(static_cast<char>(0x80 | 127));
        for (int i = 7; i >= 0; --i) {
            frame.append(static_cast<char>((len >> (8 * i)) & 0xFF));
        }
    }

    // Masking key (4 random bytes)
    quint8 maskKey[4];
    maskKey[0] = static_cast<quint8>(QRandomGenerator::global()->bounded(256));
    maskKey[1] = static_cast<quint8>(QRandomGenerator::global()->bounded(256));
    maskKey[2] = static_cast<quint8>(QRandomGenerator::global()->bounded(256));
    maskKey[3] = static_cast<quint8>(QRandomGenerator::global()->bounded(256));
    frame.append(reinterpret_cast<const char*>(maskKey), 4);

    // Masked payload
    for (quint64 i = 0; i < len; ++i) {
        frame.append(static_cast<char>(payload[i] ^ maskKey[i % 4]));
    }

    return frame;
}

// 发送 Pong 帧回复服务端 Ping
void WsClient::sendPong() {
    sendFrame(QByteArray(), WS_OPCODE_PONG);
}

// 发送 JSON 格式的文本帧
void WsClient::sendJson(const QJsonObject &obj) {
    if (!isConnected()) return;
    QJsonDocument doc(obj);
    QByteArray data = doc.toJson(QJsonDocument::Compact);

    QString type = obj["type"].toString();
    // heartbeat 不打印，其他消息打印
    if (type != "heartbeat") {
        Logger::instance().wsSend(QString::fromUtf8(data));
    }

    sendFrame(data, WS_OPCODE_TEXT);
}

// 心跳超时：发送心跳包
void WsClient::onHeartbeatTimeout() {
    sendHeartbeat();
}

// 重连定时器：尝试重新连接（超过最大次数则停止）
void WsClient::onReconnectTimeout() {
    m_reconnectAttempts++;
    if (m_reconnectAttempts > m_maxReconnectAttempts) {
        Logger::instance().wsEvent("RECONNECT_MAX_RETRIES");
        m_reconnectTimer->stop();
        return;
    }
    Logger::instance().wsEvent("RECONNECT", QString("attempt %1/%2").arg(m_reconnectAttempts).arg(m_maxReconnectAttempts));
    m_handshakeDone = false;
    m_parseState = ParseHeader;
    if (m_useSsl) {
        m_socket->connectToHostEncrypted(m_host, m_port);
    } else {
        m_socket->connectToHost(m_host, m_port);
    }
}

// 指数退避重连策略（1s -> 30s 上限）
void WsClient::tryReconnect() {
    m_reconnectInterval = qMin(m_reconnectInterval * 2, m_maxReconnectInterval);
    m_reconnectTimer->start(m_reconnectInterval);
}

// 构造消息帧并发送
void WsClient::sendMessage(quint64 toUid, int msgType,
                            const QString &content, const QString &fileUrl) {
    QJsonObject data;
    data["to_uid"] = static_cast<qint64>(toUid);
    data["msg_type"] = msgType;
    data["content"] = content;
    data["file_url"] = fileUrl;

    QJsonObject msg;
    msg["type"] = "message";
    msg["seq"] = ++m_seqCounter;
    msg["data"] = data;
    msg["timestamp"] = QDateTime::currentSecsSinceEpoch();

    sendJson(msg);
}

// 发送消息已接收确认
void WsClient::sendAck(const QString &msgId) {
    QJsonObject data;
    data["msg_id"] = msgId;

    QJsonObject msg;
    msg["type"] = "ack";
    msg["data"] = data;

    sendJson(msg);
}

// 构造心跳帧
void WsClient::sendHeartbeat() {
    QJsonObject msg;
    msg["type"] = "heartbeat";
    sendJson(msg);
}

// 构造"正在输入"通知帧
void WsClient::sendTyping(quint64 toUid) {
    QJsonObject data;
    data["to_uid"] = static_cast<qint64>(toUid);

    QJsonObject msg;
    msg["type"] = "typing";
    msg["data"] = data;

    sendJson(msg);
}
