#include "rounded_image.h"
#include <QPainter>
#include <QPainterPath>

RoundedImage::RoundedImage(QWidget *parent)
    : QLabel(parent)
    , m_radius(50)
{
}

// 设置 QPixmap 图片并触发重绘
void RoundedImage::setImage(const QPixmap &pixmap, int radius) {
    m_originalPixmap = pixmap;
    m_radius = radius;
    update();
}

// 从文件加载图片并触发重绘
void RoundedImage::setImage(const QString &filePath, int radius) {
    m_originalPixmap = QPixmap(filePath);
    m_radius = radius;
    update();
}

// 重写绘制事件：将图片按圆角矩形裁剪后绘制
void RoundedImage::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event)
    if (m_originalPixmap.isNull()) return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QPixmap scaled = m_originalPixmap.scaled(size(), Qt::KeepAspectRatioByExpanding,
                                              Qt::SmoothTransformation);

    QRect targetRect = rect();
    QPainterPath path;
    path.addRoundedRect(targetRect, m_radius, m_radius);
    painter.setClipPath(path);

    painter.drawPixmap(targetRect, scaled);
}
