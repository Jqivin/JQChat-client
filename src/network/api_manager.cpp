#include "api_manager.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QUrlQuery>
#include <functional>

ApiManager* ApiManager::s_instance = nullptr;

ApiManager* ApiManager::instance() {
    if (!s_instance) {
        s_instance = new ApiManager();
    }
    return s_instance;
}

// 构造函数：创建 HTTP 和 WebSocket 客户端并连接信号
ApiManager::ApiManager(QObject *parent)
    : QObject(parent)
    , m_httpClient(new HttpClient(this))
    , m_wsClient(new WsClient(this))
{
    connect(m_httpClient, &HttpClient::requestFinished, this, [this](int code, const QJsonObject &resp) {
        Q_UNUSED(code)
        Q_UNUSED(resp)
    });

    connect(m_wsClient, &WsClient::connected, this, &ApiManager::wsConnected);
    connect(m_wsClient, &WsClient::disconnected, this, &ApiManager::wsDisconnected);
    connect(m_wsClient, &WsClient::friendRequestReceived, this, &ApiManager::friendRequestReceived);
}

void ApiManager::setBaseUrl(const QString &url) {
    m_baseUrl = url;
    m_httpClient->setBaseUrl(url);
}

void ApiManager::setWsUrl(const QString &url) {
    m_wsUrl = url;
}

void ApiManager::setSelfInfo(const UserInfo &info) {
    m_selfInfo = info;
}

void ApiManager::setToken(const QString &token) {
    m_token = token;
    m_httpClient->setToken(token);
}

// 统一的 API 响应处理：code == 0 时调用成功回调，否则调用失败回调
void ApiManager::handleResponse(const QJsonObject &resp,
                                 std::function<void(const QJsonObject &)> onSuccess,
                                 std::function<void(int, const QString &)> onError) {
    int code = resp["code"].toInt();
    QString msg = resp["message"].toString();

    if (code == 0) {
        onSuccess(resp["data"].toObject());
    } else {
        onError(code, msg);
    }
}

// 发送邮箱验证码
void ApiManager::sendVerifyCode(const QString &email, const QString &scene) {
    QJsonObject body;
    body["email"] = email;
    body["scene"] = scene;

    connect(m_httpClient, &HttpClient::requestFinished, this, [this](int code, const QJsonObject &resp) {
        Q_UNUSED(code)
        handleResponse(resp,
            [this](const QJsonObject &) {
                emit verifyCodeSent(true, "发送成功");
            },
            [this](int, const QString &msg) {
                emit verifyCodeSent(false, msg);
            });
    }, Qt::SingleShotConnection);

    m_httpClient->post("/api/v1/verify/send", body);
}

// 用户注册：成功时保存 Token 和用户信息
void ApiManager::registerUser(const QString &email, const QString &password,
                               const QString &verifyCode, const QString &nickname) {
    QJsonObject body;
    body["email"] = email;
    body["password"] = password;
    body["verify_code"] = verifyCode;
    body["nickname"] = nickname;

    connect(m_httpClient, &HttpClient::requestFinished, this, [this](int code, const QJsonObject &resp) {
        Q_UNUSED(code)
        handleResponse(resp,
            [this](const QJsonObject &data) {
                m_token = data["token"].toString();
                m_httpClient->setToken(m_token);
                QJsonObject info = data["user_info"].toObject();
                m_selfInfo.user_id = info["id"].toVariant().toULongLong();
                m_selfInfo.email = info["email"].toString();
                m_selfInfo.nickname = info["nickname"].toString();
                m_selfInfo.avatar_url = info["avatar_url"].toString();
                emit registerResult(true, "注册成功");
            },
            [this](int, const QString &msg) {
                emit registerResult(false, msg);
            });
    }, Qt::SingleShotConnection);

    m_httpClient->post("/api/v1/user/register", body);
}

// 密码登录
void ApiManager::loginByPassword(const QString &email, const QString &password) {
    QJsonObject body;
    body["email"] = email;
    body["password"] = password;

    connect(m_httpClient, &HttpClient::requestFinished, this, [this](int code, const QJsonObject &resp) {
        Q_UNUSED(code)
        handleResponse(resp,
            [this](const QJsonObject &data) {
                m_token = data["token"].toString();
                m_httpClient->setToken(m_token);
                QJsonObject info = data["user_info"].toObject();
                m_selfInfo.user_id = info["id"].toVariant().toULongLong();
                m_selfInfo.email = info["email"].toString();
                m_selfInfo.nickname = info["nickname"].toString();
                m_selfInfo.avatar_url = info["avatar_url"].toString();
                emit loginResult(true, "登录成功");
            },
            [this](int, const QString &msg) {
                emit loginResult(false, msg);
            });
    }, Qt::SingleShotConnection);

    m_httpClient->post("/api/v1/user/login/password", body);
}

// 验证码登录
void ApiManager::loginByVerifyCode(const QString &email, const QString &verifyCode) {
    QJsonObject body;
    body["email"] = email;
    body["verify_code"] = verifyCode;

    connect(m_httpClient, &HttpClient::requestFinished, this, [this](int code, const QJsonObject &resp) {
        Q_UNUSED(code)
        handleResponse(resp,
            [this](const QJsonObject &data) {
                m_token = data["token"].toString();
                m_httpClient->setToken(m_token);
                QJsonObject info = data["user_info"].toObject();
                m_selfInfo.user_id = info["id"].toVariant().toULongLong();
                m_selfInfo.email = info["email"].toString();
                m_selfInfo.nickname = info["nickname"].toString();
                m_selfInfo.avatar_url = info["avatar_url"].toString();
                emit loginResult(true, "登录成功");
            },
            [this](int, const QString &msg) {
                emit loginResult(false, msg);
            });
    }, Qt::SingleShotConnection);

    m_httpClient->post("/api/v1/user/login/verify", body);
}

// 上传头像文件
void ApiManager::uploadAvatar(const QString &filePath) {
    connect(m_httpClient, &HttpClient::requestFinished, this, [this](int code, const QJsonObject &resp) {
        Q_UNUSED(code)
        handleResponse(resp,
            [this](const QJsonObject &data) {
                m_selfInfo.avatar_url = data["file_url"].toString();
                emit userInfoReady(m_selfInfo);
            },
            [this](int, const QString &) {});
    }, Qt::SingleShotConnection);

    m_httpClient->uploadFile("/api/v1/user/avatar", filePath, "avatar");
}

// 获取指定用户的详细信息
void ApiManager::getUserInfo(quint64 uid) {
    QUrlQuery params;
    params.addQueryItem("uid", QString::number(uid));

    connect(m_httpClient, &HttpClient::requestFinished, this, [this](int code, const QJsonObject &resp) {
        Q_UNUSED(code)
        handleResponse(resp,
            [this](const QJsonObject &data) {
                UserInfo info;
                info.user_id = data["user_id"].toVariant().toULongLong();
                info.email = data["email"].toString();
                info.nickname = data["nickname"].toString();
                info.avatar_url = data["avatar_url"].toString();
                emit userInfoReady(info);
            },
            [this](int, const QString &) {});
    }, Qt::SingleShotConnection);

    m_httpClient->get("/api/v1/user/info", params);
}

// 按邮箱搜索好友
void ApiManager::searchFriend(const QString &email) {
    QUrlQuery params;
    params.addQueryItem("email", email);

    connect(m_httpClient, &HttpClient::requestFinished, this, [this](int code, const QJsonObject &resp) {
        Q_UNUSED(code)
        handleResponse(resp,
            [this](const QJsonObject &data) {
                FriendInfo info;
                info.user_id = data["user_id"].toVariant().toULongLong();
                info.email = data["email"].toString();
                info.nickname = data["nickname"].toString();
                info.avatar_url = data["avatar_url"].toString();
                emit friendSearched(true, info);
            },
            [this](int, const QString &) {
                emit friendSearched(false, FriendInfo());
            });
    }, Qt::SingleShotConnection);

    m_httpClient->get("/api/v1/friend/search", params);
}

// 发送好友请求
void ApiManager::addFriend(quint64 friendId, const QString &remark) {
    QJsonObject body;
    body["friend_id"] = static_cast<qint64>(friendId);
    body["remark"] = remark;

    connect(m_httpClient, &HttpClient::requestFinished, this, [this](int code, const QJsonObject &resp) {
        Q_UNUSED(code)
        handleResponse(resp,
            [this](const QJsonObject &) {
                emit friendAdded(true, "好友请求已发送");
            },
            [this](int, const QString &msg) {
                emit friendAdded(false, msg);
            });
    }, Qt::SingleShotConnection);

    m_httpClient->post("/api/v1/friend/add", body);
}

// 获取好友列表
void ApiManager::getFriendList() {
    connect(m_httpClient, &HttpClient::requestFinished, this, [this](int code, const QJsonObject &resp) {
        Q_UNUSED(code)
        if (resp["code"].toInt() == 0) {
            QJsonArray arr = resp["data"].toArray();
            QList<FriendInfo> list;
            for (const auto &v : arr) {
                QJsonObject obj = v.toObject();
                FriendInfo info;
                info.user_id = obj["user_id"].toVariant().toULongLong();
                info.email = obj["email"].toString();
                info.nickname = obj["nickname"].toString();
                info.avatar_url = obj["avatar_url"].toString();
                info.remark = obj["remark"].toString();
                info.online = obj["online"].toBool();
                list.append(info);
            }
            emit friendListReady(true, list);
        } else {
            emit friendListReady(false, QList<FriendInfo>());
        }
    }, Qt::SingleShotConnection);

    m_httpClient->get("/api/v1/friend/list");
}

// 处理好友请求（接受/拒绝）
void ApiManager::handleFriendRequest(quint64 friendId, const QString &action) {
    QJsonObject body;
    body["friend_id"] = static_cast<qint64>(friendId);
    body["action"] = action;

    connect(m_httpClient, &HttpClient::requestFinished, this, [this](int code, const QJsonObject &resp) {
        Q_UNUSED(code)
        handleResponse(resp,
            [this](const QJsonObject &) {
                emit friendRequestHandled(true);
            },
            [this](int, const QString &) {
                emit friendRequestHandled(false);
            });
    }, Qt::SingleShotConnection);

    m_httpClient->put("/api/v1/friend/handle", body);
}

void ApiManager::deleteFriend(quint64 friendId) {
    QJsonObject body;
    body["friend_id"] = static_cast<qint64>(friendId);
    m_httpClient->del("/api/v1/friend/delete", body);
}

// 获取历史消息（分页：beforeId 向前翻页）
void ApiManager::getHistoryMessages(quint64 targetUid, quint64 beforeId, int limit) {
    QUrlQuery params;
    params.addQueryItem("target_uid", QString::number(targetUid));
    params.addQueryItem("before", QString::number(beforeId));
    params.addQueryItem("limit", QString::number(limit));

    connect(m_httpClient, &HttpClient::requestFinished, this, [this, targetUid](int code, const QJsonObject &resp) {
        Q_UNUSED(code)
        if (resp["code"].toInt() == 0) {
            QJsonArray arr = resp["data"].toArray();
            QList<MessageData> msgs;
            for (const auto &v : arr) {
                QJsonObject obj = v.toObject();
                MessageData msg;
                msg.msg_id = obj["msg_id"].toString();
                msg.from_uid = obj["from_uid"].toVariant().toULongLong();
                msg.to_uid = obj["to_uid"].toVariant().toULongLong();
                msg.msg_type = obj["msg_type"].toInt();
                msg.content = obj["content"].toString();
                msg.file_url = obj["file_url"].toString();
                msg.created_at = QDateTime::fromString(obj["created_at"].toString(), Qt::ISODate);
                msgs.append(msg);
            }
            emit historyMessagesReady(targetUid, msgs);
        } else {
            emit historyMessagesReady(targetUid, QList<MessageData>());
        }
    }, Qt::SingleShotConnection);

    m_httpClient->get("/api/v1/message/history", params);
}

// 获取会话列表
void ApiManager::getConversations() {
    connect(m_httpClient, &HttpClient::requestFinished, this, [this](int code, const QJsonObject &resp) {
        Q_UNUSED(code)
        if (resp["code"].toInt() == 0) {
            QJsonArray arr = resp["data"].toArray();
            QList<ConversationData> convs;
            for (const auto &v : arr) {
                QJsonObject obj = v.toObject();
                ConversationData conv;
                conv.target_uid = obj["target_uid"].toVariant().toULongLong();
                conv.last_msg = obj["last_msg"].toString();
                conv.last_time = QDateTime::fromString(obj["last_time"].toString(), Qt::ISODate);
                conv.unread = obj["unread"].toInt();
                convs.append(conv);
            }
            emit conversationsReady(convs);
        } else {
            emit conversationsReady(QList<ConversationData>());
        }
    }, Qt::SingleShotConnection);

    m_httpClient->get("/api/v1/message/conversations");
}

// 标记会话已读
void ApiManager::markAsRead(quint64 targetUid) {
    QJsonObject body;
    body["target_uid"] = static_cast<qint64>(targetUid);

    connect(m_httpClient, &HttpClient::requestFinished, this, [this](int code, const QJsonObject &resp) {
        Q_UNUSED(code)
        handleResponse(resp,
            [this](const QJsonObject &) {
                emit messagesMarkedRead();
            },
            [this](int, const QString &) {});
    }, Qt::SingleShotConnection);

    m_httpClient->put("/api/v1/message/read", body);
}

// 建立 WebSocket 连接（需要先设置 URL 和 Token）
void ApiManager::connectWebSocket() {
    if (m_wsUrl.isEmpty() || m_token.isEmpty()) return;
    m_wsClient->connectToServer(m_wsUrl, m_token);
}

// 断开 WebSocket 连接
void ApiManager::disconnectWebSocket() {
    m_wsClient->disconnect();
}
