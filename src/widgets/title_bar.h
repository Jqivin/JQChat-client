#ifndef TITLE_BAR_H
#define TITLE_BAR_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QPoint>

// 自定义标题栏：图标 + 标题 + 最小化/最大化/关闭按钮，支持拖拽移动窗口
class TitleBar : public QWidget {
    Q_OBJECT
public:
    explicit TitleBar(QWidget *parent = nullptr);

    void setTitle(const QString &title);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    QLabel *m_titleLabel;
    QPushButton *m_minBtn;
    QPushButton *m_maxBtn;
    QPushButton *m_closeBtn;
    QPoint m_dragStart;
};

#endif // TITLE_BAR_H
