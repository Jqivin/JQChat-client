#ifndef LOGIN_VIEWMODEL_H
#define LOGIN_VIEWMODEL_H

#include <QObject>
#include "network/api_manager.h"

// 登录/注册 ViewModel：封装登录业务逻辑，连接 ApiManager 结果信号到 UI
class LoginViewModel : public QObject {
    Q_OBJECT
public:
    explicit LoginViewModel(QObject *parent = nullptr);

    // 发送验证码
    void sendVerifyCode(const QString &email, const QString &scene);
    // 注册新用户
    void registerUser(const QString &email, const QString &password,
                      const QString &verifyCode, const QString &nickname);
    // 密码登录
    void loginByPassword(const QString &email, const QString &password);
    // 验证码登录
    void loginByVerifyCode(const QString &email, const QString &verifyCode);

signals:
    void verifyCodeResult(bool success, const QString &msg);
    void loginResult(bool success, const QString &msg);
    void registerResult(bool success, const QString &msg);

private slots:
    void onVerifyCodeSent(bool success, const QString &msg);
    void onLoginResult(bool success, const QString &msg);
    void onRegisterResult(bool success, const QString &msg);

private:
    ApiManager *m_api;
};

#endif // LOGIN_VIEWMODEL_H
