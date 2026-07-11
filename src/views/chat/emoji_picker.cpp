#include "emoji_picker.h"

// 构造函数：初始化网格布局并填充表情按钮
EmojiPicker::EmojiPicker(QWidget *parent)
    : QWidget(parent)
{
    m_grid = new QGridLayout(this);
    m_grid->setSpacing(5);
    setupEmojis();
}

// 创建 24 个常用 Emoji 按钮，每行 8 个
void EmojiPicker::setupEmojis() {
    QStringList emojis = {
        "😀", "😂", "🤣", "😊", "😍", "🤔", "😎", "🙌",
        "👍", "👎", "❤️", "🔥", "💯", "🎉", "✅", "❌",
        "😢", "😡", "🥺", "😴", "🤗", "😱", "🤩", "😜"
    };

    int col = 0, row = 0;
    for (const auto &emoji : emojis) {
        auto *btn = new QPushButton(emoji);
        btn->setFixedSize(40, 40);
        btn->setStyleSheet("QPushButton { font-size: 20px; border: none; }"
                           "QPushButton:hover { background: #eee; border-radius: 5px; }");
        connect(btn, &QPushButton::clicked, this, [this, emoji]() {
            emit emojiSelected(emoji);
        });
        m_grid->addWidget(btn, row, col);
        col++;
        if (col >= 8) {
            col = 0;
            row++;
        }
    }
}
