#include "contact_viewmodel.h"
#include "network/api_manager.h"

// 构造函数：连接好友相关的 API 信号和 WebSocket 在线状态信号
ContactViewModel::ContactViewModel(QObject *parent)
    : QObject(parent)
    , m_api(ApiManager::instance())
    , m_friendModel(new FriendModel(this))
{
    connect(m_api, &ApiManager::friendSearched,
            this, &ContactViewModel::onFriendSearched);
    connect(m_api, &ApiManager::friendAdded,
            this, &ContactViewModel::onFriendAdded);
    connect(m_api, &ApiManager::friendListReady,
            this, &ContactViewModel::onFriendListReady);
    connect(m_api->wsClient(), &WsClient::friendOnlineChanged,
            this, &ContactViewModel::onFriendOnlineChanged);
}

void ContactViewModel::searchByEmail(const QString &email) {
    m_api->searchFriend(email);
}

void ContactViewModel::sendFriendRequest(quint64 uid, const QString &remark) {
    m_api->addFriend(uid, remark);
}

void ContactViewModel::loadFriendList() {
    m_api->getFriendList();
}

void ContactViewModel::handleRequest(quint64 uid, bool accept) {
    m_api->handleFriendRequest(uid, accept ? "accept" : "reject");
}

void ContactViewModel::deleteFriend(quint64 uid) {
    m_api->deleteFriend(uid);
}

void ContactViewModel::onFriendSearched(bool found, const FriendInfo &info) {
    emit searchResult(info, found);
}

void ContactViewModel::onFriendAdded(bool success, const QString &msg) {
    emit friendRequestAdded(success, msg);
}

void ContactViewModel::onFriendListReady(bool success, const QList<FriendInfo> &list) {
    if (success) {
        m_friendModel->setFriendList(list);
    }
    emit friendListLoaded(success);
}

void ContactViewModel::onFriendOnlineChanged(quint64 uid, bool online) {
    m_friendModel->setOnline(uid, online);
}
