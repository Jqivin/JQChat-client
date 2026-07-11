#ifndef ROUNDED_IMAGE_H
#define ROUNDED_IMAGE_H

#include <QLabel>
#include <QPixmap>

// 圆角图片控件：使用 QPainterPath 绘制圆角矩形
class RoundedImage : public QLabel {
    Q_OBJECT
public:
    explicit RoundedImage(QWidget *parent = nullptr);

    // 设置 QPixmap 图片并指定圆角半径
    void setImage(const QPixmap &pixmap, int radius = 50);
    // 从文件加载图片并指定圆角半径
    void setImage(const QString &filePath, int radius = 50);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QPixmap m_originalPixmap;
    int m_radius;
};

#endif // ROUNDED_IMAGE_H
