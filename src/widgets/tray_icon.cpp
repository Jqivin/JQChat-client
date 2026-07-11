#include "tray_icon.h"
#include <QIcon>
#include <QApplication>

// 构造函数：创建托盘图标、右键菜单，连接双击显示信号
TrayIcon::TrayIcon(QObject *parent)
    : QObject(parent)
    , m_blinkTimer(new QTimer(this))
    , m_blinkState(true)
{
    // 使用纯色图标（暂缺图标文件）
    QPixmap pixmap(16, 16);
    pixmap.fill(QColor("#1197fa"));
    QIcon icon(pixmap);

    m_trayIcon = new QSystemTrayIcon(icon, qobject_cast<QWidget*>(parent));

    m_menu = new QMenu;
    m_showAction = m_menu->addAction("显示窗口");
    m_menu->addSeparator();
    m_quitAction = m_menu->addAction("退出");
    m_trayIcon->setContextMenu(m_menu);

    m_trayIcon->show();

    connect(m_showAction, &QAction::triggered, this, &TrayIcon::showWindow);
    connect(m_quitAction, &QAction::triggered, qApp, &QApplication::quit);
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::DoubleClick) {
            emit showWindow();
        }
    });
    connect(m_blinkTimer, &QTimer::timeout, this, &TrayIcon::onBlinkTick);
}

bool TrayIcon::isVisible() const {
    return m_trayIcon->isVisible();
}

// 显示系统气泡提示（3秒自动消失）
void TrayIcon::showMessage(const QString &title, const QString &message) {
    m_trayIcon->showMessage(title, message,
                            QSystemTrayIcon::Information, 3000);
}

// 启动图标闪烁（500ms 间隔切换可见性）
void TrayIcon::startBlink() {
    if (!m_blinkTimer->isActive()) {
        m_blinkState = true;
        m_blinkTimer->start(500);
    }
}

// 停止闪烁并确保图标可见
void TrayIcon::stopBlink() {
    m_blinkTimer->stop();
    m_trayIcon->setVisible(true);
}

// 切换图标的可见状态来实现闪烁效果
void TrayIcon::onBlinkTick() {
    m_blinkState = !m_blinkState;
    m_trayIcon->setVisible(m_blinkState);
}
