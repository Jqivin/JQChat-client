#ifndef PROTO_H
#define PROTO_H

#include <QString>
#include <QJsonObject>
#include <QDateTime>

// 用户信息数据结构
struct UserInfo {
    quint64 user_id = 0;
    QString email;
    QString nickname;
    QString avatar_url;
};

// 好友信息数据结构（含在线状态）
struct FriendInfo {
    quint64 user_id = 0;
    QString email;
    QString nickname;
    QString avatar_url;
    QString remark;
    bool online = false;
};

// 消息数据结构
struct MessageData {
    QString msg_id;
    quint64 from_uid = 0;
    quint64 to_uid = 0;
    int msg_type = 0; // 消息类型：1:文字 2:图片 3:表情
    QString content;
    QString file_url;
    QDateTime created_at;
    int status = 0; // 消息状态：0:发送中 1:已发送 2:已读
};

// 会话概要数据结构
struct ConversationData {
    quint64 target_uid = 0;
    QString last_msg;
    QDateTime last_time;
    int unread = 0;
};

// 消息类型枚举
enum MsgType {
    MSG_TEXT = 1,
    MSG_IMAGE = 2,
    MSG_EMOJI = 3,
    MSG_SYSTEM = 4
};

// 消息状态枚举
enum MsgStatus {
    MSG_SENDING = 0,
    MSG_SENT = 1,
    MSG_READ = 2
};

#endif // PROTO_H
