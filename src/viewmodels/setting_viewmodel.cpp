#include "setting_viewmodel.h"

SettingViewModel::SettingViewModel(QObject *parent)
    : QObject(parent)
    , m_settings("JQChat", "JQChat")
{
}

bool SettingViewModel::isPopupNotifyEnabled() const {
    return m_settings.value("notify/popup", true).toBool();
}

void SettingViewModel::setPopupNotifyEnabled(bool enabled) {
    m_settings.setValue("notify/popup", enabled);
    emit notifySettingChanged();
}

bool SettingViewModel::isSoundEnabled() const {
    return m_settings.value("notify/sound", true).toBool();
}

void SettingViewModel::setSoundEnabled(bool enabled) {
    m_settings.setValue("notify/sound", enabled);
}

QString SettingViewModel::serverUrl() const {
    return m_settings.value("server/url", "http://localhost:8080").toString();
}

void SettingViewModel::setServerUrl(const QString &url) {
    m_settings.setValue("server/url", url);
}

void SettingViewModel::saveLoginInfo(const QString &email, const QString &token) {
    m_settings.setValue("login/email", email);
    m_settings.setValue("login/token", token);
}

QString SettingViewModel::savedEmail() const {
    return m_settings.value("login/email").toString();
}

QString SettingViewModel::savedToken() const {
    return m_settings.value("login/token").toString();
}

void SettingViewModel::clearLoginInfo() {
    m_settings.remove("login/email");
    m_settings.remove("login/token");
}
