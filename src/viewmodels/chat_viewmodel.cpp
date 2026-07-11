#include "chat_viewmodel.h"
#include "network/api_manager.h"
#include <QFileInfo>
#include <QMimeDatabase>

// 构造函数：连接 WebSocket 消息信号和历史消息信号
ChatViewModel::ChatViewModel(QObject *parent)
    : QObject(parent)
    , m_api(ApiManager::instance())
    , m_msgModel(new MessageModel(this))
{
    m_msgModel->setSelfUid(m_api->selfInfo().user_id);
    connect(m_api->wsClient(), &WsClient::messageReceived,
            this, &ChatViewModel::onMessageReceived);
    connect(m_api->wsClient(), &WsClient::typingReceived,
            this, &ChatViewModel::onTypingReceived);
    connect(m_api, &ApiManager::historyMessagesReady,
            this, &ChatViewModel::onHistoryReady);
}

// 创建本地消息对象并发送文字消息
void ChatViewModel::sendTextMessage(quint64 toUid, const QString &text) {
    if (text.trimmed().isEmpty()) return;

    MessageData localMsg;
    localMsg.msg_id = QString::number(QDateTime::currentMSecsSinceEpoch());
    localMsg.from_uid = m_api->selfInfo().user_id;
    localMsg.to_uid = toUid;
    localMsg.msg_type = MSG_TEXT;
    localMsg.content = text;
    localMsg.created_at = QDateTime::currentDateTime();
    localMsg.status = MSG_SENDING;

    m_msgModel->addMessage(localMsg);
    emit messageSent(localMsg);

    m_api->wsClient()->sendMessage(toUid, MSG_TEXT, text, "");
}

// 创建本地消息对象并发送图片消息
void ChatViewModel::sendImageMessage(quint64 toUid, const QString &filePath) {
    QFileInfo fi(filePath);
    if (!fi.exists()) return;

    MessageData localMsg;
    localMsg.msg_id = QString::number(QDateTime::currentMSecsSinceEpoch());
    localMsg.from_uid = m_api->selfInfo().user_id;
    localMsg.to_uid = toUid;
    localMsg.msg_type = MSG_IMAGE;
    localMsg.content = fi.fileName();
    localMsg.file_url = filePath;     // 本地图片路径，用于发送方立即展示
    localMsg.created_at = QDateTime::currentDateTime();
    localMsg.status = MSG_SENDING;

    m_msgModel->addMessage(localMsg);
    emit messageSent(localMsg);

    m_api->wsClient()->sendMessage(toUid, MSG_IMAGE, fi.fileName(), filePath);
}

// 创建本地消息对象并发送表情消息
void ChatViewModel::sendEmojiMessage(quint64 toUid, const QString &emojiCode) {
    MessageData localMsg;
    localMsg.msg_id = QString::number(QDateTime::currentMSecsSinceEpoch());
    localMsg.from_uid = m_api->selfInfo().user_id;
    localMsg.to_uid = toUid;
    localMsg.msg_type = MSG_EMOJI;
    localMsg.content = emojiCode;
    localMsg.file_url = emojiCode;    // emoji 代码也存一份，用于发送方展示
    localMsg.created_at = QDateTime::currentDateTime();
    localMsg.status = MSG_SENDING;

    m_msgModel->addMessage(localMsg);
    emit messageSent(localMsg);

    m_api->wsClient()->sendMessage(toUid, MSG_EMOJI, emojiCode, "");
}

// 清空本地消息后从服务端加载历史消息
void ChatViewModel::loadHistoryMessages(quint64 targetUid) {
    m_msgModel->clearMessages(targetUid);
    m_lastMessageId[targetUid] = 0;
    m_api->getHistoryMessages(targetUid, 0, 20);
}

// 加载更早的历史消息（分页，beforeId 为最早消息的时间戳）
void ChatViewModel::loadMoreMessages(quint64 targetUid) {
    quint64 beforeId = m_lastMessageId.value(targetUid, 0);
    m_api->getHistoryMessages(targetUid, beforeId, 20);
}

void ChatViewModel::markAsRead(quint64 targetUid) {
    m_api->markAsRead(targetUid);
}

void ChatViewModel::sendTyping(quint64 toUid) {
    m_api->wsClient()->sendTyping(toUid);
}

void ChatViewModel::onMessageReceived(const MessageData &msg) {
    m_msgModel->addMessage(msg);
}

void ChatViewModel::onHistoryReady(quint64 targetUid, const QList<MessageData> &msgs) {
    m_msgModel->addMessages(targetUid, msgs);
    if (!msgs.isEmpty()) {
        m_lastMessageId[targetUid] = msgs.first().created_at.toMSecsSinceEpoch();
    }
    emit historyLoaded(targetUid);
}

void ChatViewModel::onTypingReceived(quint64 fromUid) {
    emit typingReceived(fromUid);
}
