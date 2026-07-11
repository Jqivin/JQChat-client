#include "message_model.h"

MessageModel::MessageModel(QObject *parent)
    : QObject(parent)
{
}

// 添加消息到指定目标会话，触发更新和新消息信号
void MessageModel::addMessage(const MessageData &msg) {
    // 用对话对方的 ID 作为 key（不是自己的 ID）
    quint64 key = (msg.from_uid == m_selfUid) ? msg.to_uid : msg.from_uid;
    m_messages[key].append(msg);
    emit messagesUpdated(key);
    emit newMessageReceived(msg);
}

// 批量添加消息到对应会话（targetUid 为对话对方的 ID）
void MessageModel::addMessages(quint64 targetUid, const QList<MessageData> &msgs) {
    if (msgs.isEmpty()) return;
    for (const auto &m : msgs) {
        m_messages[targetUid].append(m);
    }
    emit messagesUpdated(targetUid);
}

// 清空指定会话的消息记录
void MessageModel::clearMessages(quint64 targetUid) {
    m_messages[targetUid].clear();
    emit messagesUpdated(targetUid);
}

// 根据 msg_id 更新消息状态（发送中/已发送/已读）
void MessageModel::updateMessageStatus(const QString &msgId, int status) {
    for (auto &list : m_messages) {
        for (auto &msg : list) {
            if (msg.msg_id == msgId) {
                msg.status = status;
                emit messagesUpdated(msg.to_uid);
                return;
            }
        }
    }
}

QList<MessageData> MessageModel::messages(quint64 targetUid) const {
    return m_messages.value(targetUid);
}

// 添加新会话到列表头部，若已存在则更新
void MessageModel::addConversation(const ConversationData &conv) {
    for (int i = 0; i < m_conversations.size(); ++i) {
        if (m_conversations[i].target_uid == conv.target_uid) {
            m_conversations[i] = conv;
            emit conversationsUpdated();
            return;
        }
    }
    m_conversations.prepend(conv);
    emit conversationsUpdated();
}

// 替换整个会话列表
void MessageModel::setConversations(const QList<ConversationData> &convs) {
    m_conversations = convs;
    emit conversationsUpdated();
}

// 更新指定目标的未读消息计数
void MessageModel::updateUnread(quint64 targetUid, int count) {
    for (auto &c : m_conversations) {
        if (c.target_uid == targetUid) {
            c.unread = count;
            emit conversationsUpdated();
            return;
        }
    }
}
