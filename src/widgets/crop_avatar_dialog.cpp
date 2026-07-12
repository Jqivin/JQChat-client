#include "crop_avatar_dialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QApplication>
#include <QScreen>
#include <QPainterPath>
#include <cmath>

CropAvatarDialog::CropAvatarDialog(const QString &imagePath, QWidget *parent)
    : QDialog(parent)
{
    m_original.load(imagePath);
    if (m_original.isNull()) {
        reject();
        return;
    }

    setWindowTitle("裁剪头像");
    setMinimumSize(500, 500);
    setMaximumSize(800, 800);
    resize(600, 600);
    setStyleSheet("CropAvatarDialog { background: #333; }");
    setMouseTracking(true);

    // 底部按钮
    auto *bottomBar = new QWidget;
    bottomBar->setFixedHeight(52);
    bottomBar->setStyleSheet("background: #222;");
    auto *btnLayout = new QHBoxLayout(bottomBar);
    btnLayout->setContentsMargins(20, 0, 20, 0);

    auto *cancelBtn = new QPushButton("取消");
    cancelBtn->setStyleSheet(
        "QPushButton { background: #555; color: white; border: none;"
        " padding: 8px 28px; border-radius: 4px; font-size: 14px; }"
        "QPushButton:hover { background: #666; }");
    btnLayout->addStretch();
    btnLayout->addWidget(cancelBtn);

    auto *okBtn = new QPushButton("确定");
    okBtn->setStyleSheet(
        "QPushButton { background: #07c160; color: white; border: none;"
        " padding: 8px 28px; border-radius: 4px; font-size: 14px; }"
        "QPushButton:hover { background: #06ad56; }");
    btnLayout->addWidget(okBtn);

    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addStretch(1);
    mainLayout->addWidget(bottomBar);
}

QPixmap CropAvatarDialog::croppedPixmap() const {
    if (m_original.isNull()) return {};

    // 从缩放图坐标映射回原始图坐标
    qreal ratio = qreal(m_original.width()) / m_scaled.width();
    int rx = qRound((m_center.x() - m_radius) * ratio);
    int ry = qRound((m_center.y() - m_radius) * ratio);
    int rd = qRound(2 * m_radius * ratio);
    QRect srcRect(rx, ry, rd, rd);

    // 确保在原始图范围内
    srcRect = srcRect.intersected(m_original.rect());
    if (srcRect.isEmpty()) return {};

    QPixmap cropped = m_original.copy(srcRect);

    // 裁剪为圆形
    int size = qMin(cropped.width(), cropped.height());
    QPixmap result(size, size);
    result.fill(Qt::transparent);

    QPainter painter(&result);
    painter.setRenderHint(QPainter::Antialiasing);
    QPainterPath path;
    path.addEllipse(0, 0, size, size);
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, cropped);
    painter.end();

    // 缩放到合理大小（256x256）
    if (size > 256) {
        result = result.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    return result;
}

void CropAvatarDialog::resizeEvent(QResizeEvent *event) {
    QDialog::resizeEvent(event);
    if (m_original.isNull()) return;

    // 缩放图片适配窗口（留出底部按钮空间）
    int maxW = width() - 40;
    int maxH = height() - 80;
    m_scaled = m_original.scaled(maxW, maxH, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_scale = qreal(m_scaled.width()) / m_original.width();

    // 图片居中
    int x = (width() - m_scaled.width()) / 2;
    int y = (height() - 80 - m_scaled.height()) / 2;
    m_imageRect = QRect(x, y, m_scaled.width(), m_scaled.height());

    // 初始化裁剪圆（位于图片中央，半径为短边的 40%）
    if (m_radius < 1) {
        m_radius = qMin(m_scaled.width(), m_scaled.height()) * 0.4;
        m_center = QPointF(m_imageRect.center().x(), m_imageRect.center().y());
    }

    clampCircle();
    update();
}

void CropAvatarDialog::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor("#333"));

    if (m_scaled.isNull()) return;

    // 1. 绘制图片
    painter.drawPixmap(m_imageRect.topLeft(), m_scaled);

    // 2. 绘制半透明遮罩
    painter.save();
    QPainterPath overlay;
    overlay.addRect(rect());
    overlay.addEllipse(m_center, m_radius, m_radius);
    painter.setBrush(QColor(0, 0, 0, 160));
    painter.setPen(Qt::NoPen);
    painter.drawPath(overlay);
    painter.restore();

    // 3. 绘制裁剪圆边框
    painter.setPen(QPen(QColor("#07c160"), kBorderWidth));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(m_center, m_radius, m_radius);

    // 4. 绘制网格辅助线
    painter.setPen(QPen(QColor(255, 255, 255, 40), 1, Qt::DotLine));
    qreal third = m_radius / 3;
    painter.drawLine(QPointF(m_center.x() - m_radius, m_center.y()),
                     QPointF(m_center.x() + m_radius, m_center.y()));
    painter.drawLine(QPointF(m_center.x(), m_center.y() - m_radius),
                     QPointF(m_center.x(), m_center.y() + m_radius));
}

void CropAvatarDialog::mousePressEvent(QMouseEvent *event) {
    if (event->button() != Qt::LeftButton) return;

    QPointF pt = event->position();
    qreal dist = std::sqrt(std::pow(pt.x() - m_center.x(), 2) +
                           std::pow(pt.y() - m_center.y(), 2));

    // 判断点击在圆的边缘（缩放）还是内部（移动）
    if (qAbs(dist - m_radius) < 12) {
        m_action = Resize;
    } else if (dist < m_radius) {
        m_action = Move;
    } else {
        return;  // 点在圆外，不处理
    }

    m_dragging = true;
    m_dragStart = pt;
    m_dragCenter = m_center;
    m_dragRadius = m_radius;
}

void CropAvatarDialog::mouseMoveEvent(QMouseEvent *event) {
    QPointF pt = event->position();

    // 鼠标样式
    qreal dist = std::sqrt(std::pow(pt.x() - m_center.x(), 2) +
                           std::pow(pt.y() - m_center.y(), 2));
    if (qAbs(dist - m_radius) < 12 && !m_dragging) {
        setCursor(Qt::SizeAllCursor);
    } else if (dist < m_radius && !m_dragging) {
        setCursor(Qt::OpenHandCursor);
    } else if (!m_dragging) {
        setCursor(Qt::ArrowCursor);
    }

    if (!m_dragging) return;

    if (m_action == Move) {
        setCursor(Qt::ClosedHandCursor);
        QPointF delta = pt - m_dragStart;
        m_center = m_dragCenter + delta;
    } else if (m_action == Resize) {
        // 新半径 = 原半径 + 鼠标沿径向的位移
        QPointF d0 = m_dragStart - m_dragCenter;
        QPointF d1 = pt - m_dragCenter;
        qreal r0 = std::sqrt(d0.x() * d0.x() + d0.y() * d0.y());
        qreal r1 = std::sqrt(d1.x() * d1.x() + d1.y() * d1.y());
        m_radius = m_dragRadius + (r1 - r0);
        if (m_radius < kMinRadius) m_radius = kMinRadius;
    }

    clampCircle();
    update();
}

void CropAvatarDialog::mouseReleaseEvent(QMouseEvent *event) {
    Q_UNUSED(event)
    m_dragging = false;
    m_action = None;
    setCursor(Qt::ArrowCursor);
}

void CropAvatarDialog::clampCircle() {
    // 确保裁剪圆在图片范围内
    qreal left = m_center.x() - m_radius;
    qreal top = m_center.y() - m_radius;
    qreal right = m_center.x() + m_radius;
    qreal bottom = m_center.y() + m_radius;

    if (left < m_imageRect.left()) {
        m_center.setX(m_imageRect.left() + m_radius);
    }
    if (right > m_imageRect.right()) {
        m_center.setX(m_imageRect.right() - m_radius);
    }
    if (top < m_imageRect.top()) {
        m_center.setY(m_imageRect.top() + m_radius);
    }
    if (bottom > m_imageRect.bottom()) {
        m_center.setY(m_imageRect.bottom() - m_radius);
    }

    // 半径不能超过图片的一半
    qreal maxR = qMin(m_scaled.width(), m_scaled.height()) / 2.0;
    if (m_radius > maxR) m_radius = maxR;
}
