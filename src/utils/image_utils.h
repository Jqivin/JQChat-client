#ifndef IMAGE_UTILS_H
#define IMAGE_UTILS_H

#include <QString>
#include <QPixmap>

// 图片处理工具类：剪裁、缩放、圆角、背景模糊
class ImageUtils {
public:
    // 居中裁剪并缩放到指定尺寸（头像用）
    static QPixmap cropAndScale(const QString &filePath, int size);
    // 将图片裁剪为圆形
    static QPixmap cropToCircle(const QPixmap &pixmap);
    // 高斯模糊背景（暂未实现）
    static QPixmap blurBackground(const QPixmap &pixmap, int radius);
};

#endif // IMAGE_UTILS_H
