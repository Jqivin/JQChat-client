#ifndef SETTING_DIALOG_H
#define SETTING_DIALOG_H

#include <QDialog>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>

class SettingViewModel;

// 设置对话框：通知开关 + 声音开关 + 账号管理
class SettingDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingDialog(QWidget *parent = nullptr);

private slots:
    // 通知开关切换
    void onNotifyToggled(bool checked);
    // 退出登录
    void onLogout();
    void onClearCache();

private:
    SettingViewModel *m_viewModel;
    QCheckBox *m_popupNotifyCheck;
    QCheckBox *m_soundCheck;
    QPushButton *m_logoutBtn;
};

#endif // SETTING_DIALOG_H
