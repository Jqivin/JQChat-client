#ifndef CHAT_VIEWMODEL_H
#define CHAT_VIEWMODEL_H

#include <QObject>
#include "network/proto.h"
#include "models/message_model.h"

class ApiManager;

// 聊天 ViewModel：管理消息发送/接收、历史加载和"正在输入"状态
class ChatViewModel : public QObject {
    Q_OBJECT
public:
    explicit ChatViewModel(QObject *parent = nullptr);

    // 发送文字消息
    void sendTextMessage(quint64 toUid, const QString &text);
    // 发送图片消息
    void sendImageMessage(quint64 toUid, const QString &filePath);
    // 发送表情消息
    void sendEmojiMessage(quint64 toUid, const QString &emojiCode);
    // 加载历史消息
    void loadHistoryMessages(quint64 targetUid);
    // 加载更多历史消息（分页）
    void loadMoreMessages(quint64 targetUid);
    // 标记已读
    void markAsRead(quint64 targetUid);
    // 发送正在输入提示
    void sendTyping(quint64 toUid);

    MessageModel *messageModel() const { return m_msgModel; }

signals:
    void messageSent(const MessageData &msg);
    void historyLoaded(quint64 targetUid);
    void typingReceived(quint64 fromUid);

private slots:
    void onMessageReceived(const MessageData &msg);
    void onHistoryReady(quint64 targetUid, const QList<MessageData> &msgs);
    void onTypingReceived(quint64 fromUid);

private:
    ApiManager *m_api;
    MessageModel *m_msgModel;
    QMap<quint64, quint64> m_lastMessageId;
};

#endif // CHAT_VIEWMODEL_H
