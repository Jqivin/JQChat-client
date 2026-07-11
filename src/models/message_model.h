#ifndef MESSAGE_MODEL_H
#define MESSAGE_MODEL_H

#include <QObject>
#include <QList>
#include "network/proto.h"

// 消息与会话数据模型：管理消息列表和会话列表
class MessageModel : public QObject {
    Q_OBJECT
public:
    explicit MessageModel(QObject *parent = nullptr);

    // 设置当前用户 ID（用于正确路由消息 key）
    void setSelfUid(quint64 uid) { m_selfUid = uid; }

    // 添加单条消息
    void addMessage(const MessageData &msg);
    // 批量添加消息（targetUid 为对话对方的 ID）
    void addMessages(quint64 targetUid, const QList<MessageData> &msgs);
    // 清空指定会话的消息
    void clearMessages(quint64 targetUid);
    // 更新消息发送状态
    void updateMessageStatus(const QString &msgId, int status);
    // 获取指定会话的消息列表
    QList<MessageData> messages(quint64 targetUid) const;

    // 添加或更新会话
    void addConversation(const ConversationData &conv);
    // 替换所有会话
    void setConversations(const QList<ConversationData> &convs);
    // 更新未读消息数
    void updateUnread(quint64 targetUid, int count);
    QList<ConversationData> conversations() const { return m_conversations; }

signals:
    void messagesUpdated(quint64 targetUid);
    void conversationsUpdated();
    void newMessageReceived(const MessageData &msg);

private:
    QMap<quint64, QList<MessageData>> m_messages;
    QList<ConversationData> m_conversations;
    quint64 m_selfUid = 0;
};

#endif // MESSAGE_MODEL_H
