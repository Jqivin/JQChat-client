#include "image_utils.h"
#include <QPainter>
#include <QPainterPath>
#include <QImageReader>

// 加载图片后居中裁剪为正方形并缩放到指定尺寸
QPixmap ImageUtils::cropAndScale(const QString &filePath, int size) {
    QPixmap pixmap(filePath);
    if (pixmap.isNull()) return pixmap;

    int side = qMin(pixmap.width(), pixmap.height());
    QRect cropRect((pixmap.width() - side) / 2,
                   (pixmap.height() - side) / 2, side, side);
    QPixmap cropped = pixmap.copy(cropRect);
    return cropped.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

// 使用 QPainterPath 将图片裁成圆形
QPixmap ImageUtils::cropToCircle(const QPixmap &pixmap) {
    if (pixmap.isNull()) return pixmap;

    int side = qMin(pixmap.width(), pixmap.height());
    QPixmap result(side, side);
    result.fill(Qt::transparent);

    QPainter painter(&result);
    painter.setRenderHint(QPainter::Antialiasing);
    QPainterPath path;
    path.addEllipse(0, 0, side, side);
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, side, side, pixmap);
    return result;
}

QPixmap ImageUtils::blurBackground(const QPixmap &pixmap, int radius) {
    Q_UNUSED(radius)
    return pixmap;
}
