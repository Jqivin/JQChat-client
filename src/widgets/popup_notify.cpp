#include "popup_notify.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGuiApplication>
#include <QScreen>
#include <QMouseEvent>

// 构造函数：无边框置顶窗口，头像 + 发件人 + 消息预览
PopupNotify::PopupNotify(QWidget *parent)
    : QWidget(parent)
    , m_enabled(true)
    , m_autoCloseTimer(new QTimer(this))
{
    setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setFixedSize(300, 80);
    setStyleSheet("PopupNotify { background: white; border: 1px solid #ddd;"
                  " border-radius: 8px; }");

    auto *layout = new QHBoxLayout(this);

    m_avatar = new QLabel;
    m_avatar->setFixedSize(40, 40);
    m_avatar->setStyleSheet("background: #ddd; border-radius: 20px;");
    layout->addWidget(m_avatar);

    auto *textLayout = new QVBoxLayout;
    m_title = new QLabel;
    m_title->setStyleSheet("font-weight: bold; font-size: 13px;");
    textLayout->addWidget(m_title);

    m_content = new QLabel;
    m_content->setStyleSheet("color: #666; font-size: 12px;");
    m_content->setWordWrap(true);
    textLayout->addWidget(m_content);

    layout->addLayout(textLayout);

    m_autoCloseTimer->setSingleShot(true);
    connect(m_autoCloseTimer, &QTimer::timeout, this, &QWidget::hide);
}

// 显示通知：定位到屏幕右下角，淡入动画，3秒后自动隐藏
void PopupNotify::showNotification(const QString &sender, const QString &preview) {
    if (!m_enabled) return;

    m_title->setText(sender);
    m_content->setText(preview);

    QRect screenRect = QGuiApplication::primaryScreen()->availableGeometry();
    move(screenRect.right() - width() - 10, screenRect.bottom() - height() - 50);

    auto *anim = new QPropertyAnimation(this, "windowOpacity");
    anim->setDuration(300);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);

    show();
    raise();
    anim->start(QAbstractAnimation::DeleteWhenStopped);
    m_autoCloseTimer->start(3000);
}

// 点击通知：关闭通知并打开主窗口
void PopupNotify::mousePressEvent(QMouseEvent *event) {
    Q_UNUSED(event)
    hide();
    emit showWindow();
}

// 鼠标悬停：停止自动关闭定时器
void PopupNotify::enterEvent(QEnterEvent *event) {
    Q_UNUSED(event)
    m_autoCloseTimer->stop();
}
