#ifndef CHAT_WINDOW_H
#define CHAT_WINDOW_H

#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QLabel>
#include <QMap>
#include "network/proto.h"

class ChatViewModel;
class ChatInput;
class EmojiPicker;
class MessageItem;

// 聊天窗口：标题栏 + 消息滚动区域 + 表情选择器 + 输入栏
class ChatWindow : public QWidget {
    Q_OBJECT
public:
    explicit ChatWindow(ChatViewModel *viewModel, QWidget *parent = nullptr);

    // 设置聊天目标用户
    void setTarget(quint64 uid, const QString &name);
    // 添加单条消息到界面
    void addMessage(const MessageData &msg);
    // 清空所有消息控件
    void clearMessages();
    // 设置用户头像 URL（供 MainWindow 调用）
    void setUserAvatar(quint64 uid, const QString &url);
    void updateMessageWidths();

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onMessagesUpdated(quint64 targetUid);
    void onSendText(const QString &text);
    void onSendImage(const QString &path);
    void onSendEmoji(const QString &code);
    void onTypingReceived(quint64 fromUid);

private:
    void scrollToBottom();

    ChatViewModel *m_viewModel;
    quint64 m_targetUid = 0;

    QLabel *m_titleLabel;
    QScrollArea *m_scrollArea;
    QWidget *m_messageContainer;
    QVBoxLayout *m_messageLayout;
    ChatInput *m_chatInput;
    EmojiPicker *m_emojiPicker;
    QMap<quint64, QString> m_avatars;
};

#endif // CHAT_WINDOW_H
