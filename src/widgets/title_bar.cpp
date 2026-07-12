#include "title_bar.h"
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QMainWindow>
#include <QStyle>
#include <QIcon>
#include <QPixmap>
#include <QPalette>

// 按钮样式宏
static const char *kBtnStyle =
    "QPushButton { background: transparent; color: white; border: none;"
    " font-size: 16px; padding: 4px 10px; }"
    "QPushButton:hover { background: #3a3a3a; }";

static const char *kCloseBtnStyle =
    "QPushButton { background: transparent; color: white; border: none;"
    " font-size: 16px; padding: 4px 10px; }"
    "QPushButton:hover { background: #e81123; }";

TitleBar::TitleBar(QWidget *parent)
    : QWidget(parent)
{
    setFixedHeight(36);
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(0, 0, 0));
    setPalette(pal);

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 0, 0, 0);
    layout->setSpacing(0);

    // 标题
    m_titleLabel = new QLabel("JQChat");
    m_titleLabel->setStyleSheet("color: white; font-size: 13px;");
    layout->addWidget(m_titleLabel);
    layout->addStretch();

    // 最小化
    m_minBtn = new QPushButton;
    m_minBtn->setFixedSize(40, 36);
    m_minBtn->setIcon(QIcon(":/images/mini"));
    m_minBtn->setIconSize(QSize(14, 14));
    m_minBtn->setStyleSheet(kBtnStyle);
    connect(m_minBtn, &QPushButton::clicked, this, [this]() {
        window()->showMinimized();
    });
    layout->addWidget(m_minBtn);

    // 最大化/还原
    m_maxBtn = new QPushButton;
    m_maxBtn->setFixedSize(40, 36);
    m_maxBtn->setIcon(QIcon(":/images/max"));
    m_maxBtn->setIconSize(QSize(14, 14));
    m_maxBtn->setStyleSheet(kBtnStyle);
    connect(m_maxBtn, &QPushButton::clicked, this, [this]() {
        if (window()->isMaximized()) {
            window()->showNormal();
        } else {
            window()->showMaximized();
        }
    });
    layout->addWidget(m_maxBtn);

    // 关闭
    m_closeBtn = new QPushButton;
    m_closeBtn->setFixedSize(40, 36);
    m_closeBtn->setIcon(QIcon(":/images/close"));
    m_closeBtn->setIconSize(QSize(14, 14));
    m_closeBtn->setStyleSheet(kCloseBtnStyle);
    connect(m_closeBtn, &QPushButton::clicked, this, [this]() {
        window()->close();
    });
    layout->addWidget(m_closeBtn);
}

void TitleBar::setTitle(const QString &title) {
    m_titleLabel->setText(title);
}

void TitleBar::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_dragStart = event->globalPosition().toPoint();
    }
}

void TitleBar::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        QPoint delta = event->globalPosition().toPoint() - m_dragStart;
        m_dragStart = event->globalPosition().toPoint();
        window()->move(window()->pos() + delta);
    }
}

void TitleBar::mouseDoubleClickEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        if (window()->isMaximized())
            window()->showNormal();
        else
            window()->showMaximized();
    }
}
