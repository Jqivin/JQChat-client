#ifndef EMOJI_PICKER_H
#define EMOJI_PICKER_H

#include <QWidget>
#include <QGridLayout>
#include <QPushButton>
#include <QStringList>

// 表情选择器：网格布局展示常用 Emoji
class EmojiPicker : public QWidget {
    Q_OBJECT
public:
    explicit EmojiPicker(QWidget *parent = nullptr);

signals:
    void emojiSelected(const QString &code);

private:
    void setupEmojis();
    QGridLayout *m_grid;
};

#endif // EMOJI_PICKER_H
