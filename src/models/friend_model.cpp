#include "friend_model.h"

FriendModel::FriendModel(QObject *parent)
    : QObject(parent)
{
}

// 替换好友列表全量数据
void FriendModel::setFriendList(const QList<FriendInfo> &list) {
    m_friends = list;
    emit friendListUpdated();
}

// 新增好友到列表末尾
void FriendModel::addFriend(const FriendInfo &info) {
    m_friends.append(info);
    emit friendListUpdated();
}

// 按用户 ID 移除好友
void FriendModel::removeFriend(quint64 userId) {
    m_friends.erase(
        std::remove_if(m_friends.begin(), m_friends.end(),
            [userId](const FriendInfo &f) { return f.user_id == userId; }),
        m_friends.end());
    emit friendListUpdated();
}

// 设置好友在线状态并通知 UI
void FriendModel::setOnline(quint64 userId, bool online) {
    for (auto &f : m_friends) {
        if (f.user_id == userId) {
            f.online = online;
            emit friendOnlineChanged(userId, online);
            return;
        }
    }
}

// 在好友列表中查找指定用户，未找到返回默认 FriendInfo
FriendInfo FriendModel::findFriend(quint64 userId) const {
    for (const auto &f : m_friends) {
        if (f.user_id == userId) return f;
    }
    return FriendInfo();
}
