#ifndef TRAY_ICON_H
#define TRAY_ICON_H

#include <QObject>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QMenu>

// 系统托盘图标：管理右键菜单、消息闪烁和气泡提示
class TrayIcon : public QObject {
    Q_OBJECT
public:
    explicit TrayIcon(QObject *parent = nullptr);
    bool isVisible() const;

    // 显示托盘消息气泡
    void showMessage(const QString &title, const QString &message);
    // 开始闪烁
    void startBlink();
    // 停止闪烁
    void stopBlink();

signals:
    void showWindow();

private slots:
    void onBlinkTick();

private:
    QSystemTrayIcon *m_trayIcon;
    QMenu *m_menu;
    QAction *m_showAction;
    QAction *m_quitAction;
    QTimer *m_blinkTimer;
    bool m_blinkState;
};

#endif // TRAY_ICON_H
