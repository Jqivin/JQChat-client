#include "setting_dialog.h"
#include "viewmodels/setting_viewmodel.h"
#include <QVBoxLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QApplication>

// 构造函数：构建通知设置和账号管理分组
SettingDialog::SettingDialog(QWidget *parent)
    : QDialog(parent)
    , m_viewModel(new SettingViewModel(this))
{
    setWindowTitle("设置");
    setFixedSize(380, 280);
    setStyleSheet("QDialog { background: #fff; }");

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 20, 24, 20);
    layout->setSpacing(16);

    auto *title = new QLabel("⚙ 设置");
    title->setStyleSheet("font-size: 18px; font-weight: bold; color: #333;");
    layout->addWidget(title);

    // 消息提醒分组
    auto *notifyGroup = new QGroupBox("消息提醒");
    auto *notifyLayout = new QVBoxLayout(notifyGroup);
    notifyLayout->setSpacing(10);

    m_popupNotifyCheck = new QCheckBox("右下角弹窗通知");
    m_popupNotifyCheck->setChecked(m_viewModel->isPopupNotifyEnabled());
    notifyLayout->addWidget(m_popupNotifyCheck);

    m_soundCheck = new QCheckBox("声音提醒");
    m_soundCheck->setChecked(m_viewModel->isSoundEnabled());
    notifyLayout->addWidget(m_soundCheck);

    layout->addWidget(notifyGroup);

    // 账号分组
    auto *accountGroup = new QGroupBox("账号");
    auto *accountLayout = new QVBoxLayout(accountGroup);

    m_logoutBtn = new QPushButton("退出登录");
    m_logoutBtn->setStyleSheet(
        "QPushButton { color: #e74c3c; background: #fff;"
        "border: 1px solid #e74c3c; border-radius: 4px;"
        "padding: 8px 16px; font-size: 13px; }"
        "QPushButton:hover { background: #fff5f5; }");
    m_logoutBtn->setCursor(Qt::PointingHandCursor);
    accountLayout->addWidget(m_logoutBtn);

    layout->addWidget(accountGroup);
    layout->addStretch();

    connect(m_popupNotifyCheck, &QCheckBox::toggled, this, &SettingDialog::onNotifyToggled);
    connect(m_soundCheck, &QCheckBox::toggled, m_viewModel, &SettingViewModel::setSoundEnabled);
    connect(m_logoutBtn, &QPushButton::clicked, this, &SettingDialog::onLogout);
}

// 通知开关变更：更新 ViewModel 设置
void SettingDialog::onNotifyToggled(bool checked) {
    m_viewModel->setPopupNotifyEnabled(checked);
}

// 确认退出登录：清除登录信息并退出应用
void SettingDialog::onLogout() {
    auto ret = QMessageBox::question(this, "退出登录",
        "确定要退出登录吗？", QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        m_viewModel->clearLoginInfo();
        close();
        QApplication::quit();
    }
}
