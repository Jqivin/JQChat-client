#ifndef USER_MODEL_H
#define USER_MODEL_H

#include <QObject>
#include "network/proto.h"

// 用户数据模型：持有当前登录用户信息并提供属性接口
class UserModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(quint64 userId READ userId NOTIFY userChanged)
    Q_PROPERTY(QString email READ email NOTIFY userChanged)
    Q_PROPERTY(QString nickname READ nickname NOTIFY userChanged)
    Q_PROPERTY(QString avatarUrl READ avatarUrl NOTIFY userChanged)

public:
    explicit UserModel(QObject *parent = nullptr);

    quint64 userId() const { return m_userInfo.user_id; }
    QString email() const { return m_userInfo.email; }
    QString nickname() const { return m_userInfo.nickname; }
    QString avatarUrl() const { return m_userInfo.avatar_url; }

    // 更新用户信息并发射 userChanged 信号
    void setUserInfo(const UserInfo &info);
    UserInfo userInfo() const { return m_userInfo; }
    // 判断是否已登录（user_id > 0）
    bool isLoggedIn() const { return m_userInfo.user_id > 0; }

signals:
    void userChanged();

private:
    UserInfo m_userInfo;
};

#endif // USER_MODEL_H
