#ifndef CONTACT_VIEWMODEL_H
#define CONTACT_VIEWMODEL_H

#include <QObject>
#include "network/proto.h"
#include "models/friend_model.h"

class ApiManager;

// 联系人 ViewModel：管理好友搜索、添加、删除和列表
class ContactViewModel : public QObject {
    Q_OBJECT
public:
    explicit ContactViewModel(QObject *parent = nullptr);

    // 按邮箱搜索好友
    void searchByEmail(const QString &email);
    // 发送好友请求
    void sendFriendRequest(quint64 uid, const QString &remark = "");
    // 加载好友列表
    void loadFriendList();
    // 处理好友请求（接受/拒绝）
    void handleRequest(quint64 uid, bool accept);
    // 删除好友
    void deleteFriend(quint64 uid);

    FriendModel *friendModel() const { return m_friendModel; }

signals:
    void searchResult(const FriendInfo &info, bool found);
    void friendRequestAdded(bool success, const QString &msg);
    void friendListLoaded(bool success);

private slots:
    void onFriendSearched(bool found, const FriendInfo &info);
    void onFriendAdded(bool success, const QString &msg);
    void onFriendListReady(bool success, const QList<FriendInfo> &list);
    void onFriendOnlineChanged(quint64 uid, bool online);

private:
    ApiManager *m_api;
    FriendModel *m_friendModel;
};

#endif // CONTACT_VIEWMODEL_H
