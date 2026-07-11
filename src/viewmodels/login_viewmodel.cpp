#include "login_viewmodel.h"

// 构造函数：连接 ApiManager 信号到内部槽函数
LoginViewModel::LoginViewModel(QObject *parent)
    : QObject(parent)
    , m_api(ApiManager::instance())
{
    connect(m_api, &ApiManager::verifyCodeSent,
            this, &LoginViewModel::onVerifyCodeSent);
    connect(m_api, &ApiManager::loginResult,
            this, &LoginViewModel::onLoginResult);
    connect(m_api, &ApiManager::registerResult,
            this, &LoginViewModel::onRegisterResult);
}

void LoginViewModel::sendVerifyCode(const QString &email, const QString &scene) {
    m_api->sendVerifyCode(email, scene);
}

void LoginViewModel::registerUser(const QString &email, const QString &password,
                                   const QString &verifyCode, const QString &nickname) {
    m_api->registerUser(email, password, verifyCode, nickname);
}

void LoginViewModel::loginByPassword(const QString &email, const QString &password) {
    m_api->loginByPassword(email, password);
}

void LoginViewModel::loginByVerifyCode(const QString &email, const QString &verifyCode) {
    m_api->loginByVerifyCode(email, verifyCode);
}

void LoginViewModel::onVerifyCodeSent(bool success, const QString &msg) {
    emit verifyCodeResult(success, msg);
}

void LoginViewModel::onLoginResult(bool success, const QString &msg) {
    emit loginResult(success, msg);
}

void LoginViewModel::onRegisterResult(bool success, const QString &msg) {
    emit registerResult(success, msg);
}
