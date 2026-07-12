#ifndef CROP_AVATAR_DIALOG_H
#define CROP_AVATAR_DIALOG_H

#include <QDialog>
#include <QPixmap>
#include <QPainter>
#include <QMouseEvent>

class QPushButton;

// 头像裁剪弹窗：圆形裁切区域，拖拽移动、边缘缩放，截取选中的圆形图片
class CropAvatarDialog : public QDialog {
    Q_OBJECT
public:
    explicit CropAvatarDialog(const QString &imagePath, QWidget *parent = nullptr);

    // 获取裁剪后的圆形图片
    QPixmap croppedPixmap() const;

private:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

    // 限制裁剪圆在图片范围内
    void clampCircle();
    // 绘制半透明遮罩层
    void drawOverlay(QPainter &painter);

    QPixmap m_original;          // 原始图片
    QPixmap m_scaled;            // 缩放后的图片
    QRect m_imageRect;           // 图片在窗口中的绘制区域

    QPointF m_center;            // 裁剪圆心（相对于图片）
    qreal m_radius;              // 裁剪圆半径

    qreal m_scale;               // 图片缩放比例

    bool m_dragging = false;     // 是否正在拖拽
    QPointF m_dragStart;         // 拖拽起始点
    QPointF m_dragCenter;        // 拖拽前圆心位置
    qreal m_dragRadius = 0;      // 拖拽前半径

    enum { None, Move, Resize } m_action = None;

    static constexpr int kMinRadius = 40;
    static constexpr int kBorderWidth = 2;
};

#endif // CROP_AVATAR_DIALOG_H
