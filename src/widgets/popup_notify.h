#ifndef POPUP_NOTIFY_H
#define POPUP_NOTIFY_H

#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QPropertyAnimation>
#include <QEnterEvent>

// 弹窗通知控件：屏幕右下角弹出新消息预览
class PopupNotify : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal windowOpacity READ windowOpacity WRITE setWindowOpacity)
public:
    explicit PopupNotify(QWidget *parent = nullptr);

    // 显示通知（淡入动画，3秒后自动关闭）
    void showNotification(const QString &sender, const QString &preview);
    // 启用/禁用通知
    void setEnabled(bool enabled) { m_enabled = enabled; }

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *event) override;

signals:
    void showWindow();

private:
    QLabel *m_avatar;
    QLabel *m_title;
    QLabel *m_content;
    QTimer *m_autoCloseTimer;
    bool m_enabled;
};

#endif // POPUP_NOTIFY_H
