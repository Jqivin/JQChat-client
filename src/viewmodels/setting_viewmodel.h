#ifndef SETTING_VIEWMODEL_H
#define SETTING_VIEWMODEL_H

#include <QObject>
#include <QSettings>

// 设置 ViewModel：管理弹窗通知、声音、服务器地址等配置
class SettingViewModel : public QObject {
    Q_OBJECT
public:
    explicit SettingViewModel(QObject *parent = nullptr);

    // 是否启用弹窗通知
    bool isPopupNotifyEnabled() const;
    void setPopupNotifyEnabled(bool enabled);

    // 是否启用声音提醒
    bool isSoundEnabled() const;
    void setSoundEnabled(bool enabled);

    // 服务器地址
    QString serverUrl() const;
    void setServerUrl(const QString &url);

    // 登录信息持久化
    void saveLoginInfo(const QString &email, const QString &token);
    QString savedEmail() const;
    QString savedToken() const;
    void clearLoginInfo();

signals:
    void notifySettingChanged();

private:
    QSettings m_settings;
};

#endif // SETTING_VIEWMODEL_H
