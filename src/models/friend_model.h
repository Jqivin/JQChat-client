#ifndef FRIEND_MODEL_H
#define FRIEND_MODEL_H

#include <QObject>
#include <QList>
#include "network/proto.h"

// 好友数据模型：管理好友列表及在线状态
class FriendModel : public QObject {
    Q_OBJECT
public:
    explicit FriendModel(QObject *parent = nullptr);

    // 替换整个好友列表
    void setFriendList(const QList<FriendInfo> &list);
    // 添加单个好友
    void addFriend(const FriendInfo &info);
    // 删除好友
    void removeFriend(quint64 userId);
    // 更新好友在线状态
    void setOnline(quint64 userId, bool online);
    QList<FriendInfo> friendList() const { return m_friends; }
    // 根据用户 ID 查找好友
    FriendInfo findFriend(quint64 userId) const;

signals:
    void friendListUpdated();
    void friendOnlineChanged(quint64 userId, bool online);

private:
    QList<FriendInfo> m_friends;
};

#endif // FRIEND_MODEL_H
