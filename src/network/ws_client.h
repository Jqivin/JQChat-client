#ifndef WS_CLIENT_H
#define WS_CLIENT_H

#include <QObject>
#include <QSslSocket>
#include <QTimer>
#include <QJsonObject>
#include <QByteArray>
#include "proto.h"

// WebSocket 客户端：基于 QTcpSocket 手动实现 WebSocket 协议
class WsClient : public QObject {
    Q_OBJECT
public:
    explicit WsClient(QObject *parent = nullptr);
    ~WsClient();

    // 连接到 WebSocket 服务器
    void connectToServer(const QString &url, const QString &token);
    // 断开连接（发送关闭帧）
    void disconnect();
    // 是否已连接并完成握手
    bool isConnected() const;

    // 发送聊天消息
    void sendMessage(quint64 toUid, int msgType,
                     const QString &content, const QString &fileUrl);
    // 发送消息确认
    void sendAck(const QString &msgId);
    // 发送心跳包
    void sendHeartbeat();
    // 发送正在输入状态
    void sendTyping(quint64 toUid);

signals:
    void connected();
    void disconnected();
    void messageReceived(const MessageData &msg);
    void friendOnlineChanged(quint64 uid, bool online);
    void typingReceived(quint64 fromUid);
    void friendRequestReceived(quint64 fromUid, const QString &fromName, const QString &remark);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onHeartbeatTimeout();
    void onReconnectTimeout();

private:
    void tryReconnect();
    void sendJson(const QJsonObject &obj);

    // WebSocket 协议实现
    void performHandshake();                // HTTP 升级握手
    void sendFrame(const QByteArray &payload, quint8 opcode);  // 发送帧
    void processFrame(const QByteArray &data, quint8 opcode);  // 处理收到的帧
    void sendPong();                        // 回复 Pong
    QByteArray encodeFrame(const QByteArray &payload, quint8 opcode);  // 编码帧

    QSslSocket *m_socket;
    bool m_useSsl = false;
    QTimer *m_heartbeatTimer;
    QTimer *m_reconnectTimer;

    QString m_host;
    int m_port;
    QString m_path;
    QString m_token;

    int m_reconnectInterval = 1000;
    int m_maxReconnectInterval = 30000;
    int m_reconnectAttempts = 0;
    int m_maxReconnectAttempts = 10;
    qint64 m_seqCounter = 0;

    // WebSocket 解析状态
    QByteArray m_readBuffer;
    bool m_handshakeDone = false;
    enum ParseState {
        ParseHeader,
        ParsePayload
    } m_parseState = ParseHeader;
    quint8 m_frameOpcode = 0;
    quint64 m_framePayloadLen = 0;
    int m_frameHeaderSize = 2;   // 帧头实际大小（含扩展长度和掩码）
    QByteArray m_framePayload;
    QByteArray m_maskKey;
    bool m_frameMasked = false;
};

#endif // WS_CLIENT_H
