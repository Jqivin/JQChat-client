#ifndef MESSAGE_ITEM_H
#define MESSAGE_ITEM_H

#include <QFrame>
#include <QLabel>
#include "network/proto.h"

// 消息气泡控件：支持文字/图片/表情三种类型，区分自己/对方
class MessageItem : public QFrame {
    Q_OBJECT
public:
    explicit MessageItem(const MessageData &msg, bool isSelf,
                         const QString &avatarUrl = "",
                         QWidget *parent = nullptr);

private:
    // 设置文字气泡样式
    void setupText(const MessageData &msg, bool isSelf);
    // 设置图片消息样式
    void setupImage(const MessageData &msg, bool isSelf);
    // 设置表情消息样式
    void setupEmoji(const MessageData &msg, bool isSelf);

    QLabel *m_contentLabel;
    QLabel *m_timeLabel;
};

#endif // MESSAGE_ITEM_H
