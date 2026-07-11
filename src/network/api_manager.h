#ifndef API_MANAGER_H
#define API_MANAGER_H

#include <QObject>
#include <QJsonObject>
#include "http_client.h"
#include "ws_client.h"
#include "proto.h"

// API 管理类：封装所有 HTTP 和 WebSocket 接口，提供信号驱动的结果回调
class ApiManager : public QObject {
    Q_OBJECT
public:
    // 获取单例实例
    static ApiManager* instance();

    // 设置服务器地址
    void setBaseUrl(const QString &url);
    void setWsUrl(const QString &url);
    // 设置鉴权 Token
    void setToken(const QString &token);

    // ============ 认证接口 ============
    void sendVerifyCode(const QString &email, const QString &scene);  // 发送验证码
    void registerUser(const QString &email, const QString &password,  // 注册
                      const QString &verifyCode, const QString &nickname);
    void loginByPassword(const QString &email, const QString &password);  // 密码登录
    void loginByVerifyCode(const QString &email, const QString &verifyCode);  // 验证码登录
    void uploadAvatar(const QString &filePath);  // 上传头像
    void getUserInfo(quint64 uid);  // 获取用户信息

    // ============ 好友接口 ============
    void searchFriend(const QString &email);  // 按邮箱搜索好友
    void addFriend(quint64 friendId, const QString &remark = "");  // 添加好友
    void getFriendList();  // 获取好友列表
    void handleFriendRequest(quint64 friendId, const QString &action);  // 处理好友请求
    void deleteFriend(quint64 friendId);  // 删除好友

    // ============ 消息接口 ============
    void getHistoryMessages(quint64 targetUid, quint64 beforeId, int limit);  // 历史消息
    void getConversations();  // 获取会话列表
    void markAsRead(quint64 targetUid);  // 标记已读

    // ============ WebSocket ============
    void connectWebSocket();   // 连接 WebSocket
    void disconnectWebSocket();  // 断开 WebSocket
    WsClient* wsClient() const { return m_wsClient; }

    QString token() const { return m_token; }
    UserInfo selfInfo() const { return m_selfInfo; }
    void setSelfInfo(const UserInfo &info);

signals:
    // Auth
    void verifyCodeSent(bool success, const QString &msg);
    void loginResult(bool success, const QString &msg);
    void registerResult(bool success, const QString &msg);
    void userInfoReady(const UserInfo &info);

    // Friends
    void friendSearched(bool found, const FriendInfo &info);
    void friendAdded(bool success, const QString &msg);
    void friendListReady(bool success, const QList<FriendInfo> &list);
    void friendRequestHandled(bool success);
    void friendRequestReceived(quint64 fromUid, const QString &fromName, const QString &remark);

    // Messages
    void historyMessagesReady(quint64 targetUid, const QList<MessageData> &msgs);
    void conversationsReady(const QList<ConversationData> &convs);
    void messagesMarkedRead();

    // WS
    void wsConnected();
    void wsDisconnected();

private:
    explicit ApiManager(QObject *parent = nullptr);
    static ApiManager *s_instance;

    // 统一处理响应：检查 code，分发成功/失败回调
    void handleResponse(const QJsonObject &resp,
                        std::function<void(const QJsonObject &)> onSuccess,
                        std::function<void(int, const QString &)> onError);

    HttpClient *m_httpClient;
    WsClient *m_wsClient;
    QString m_baseUrl;
    QString m_wsUrl;
    QString m_token;
    UserInfo m_selfInfo;
};

#endif // API_MANAGER_H
