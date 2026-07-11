#include "chat_input.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QKeyEvent>

// 构造函数：创建工具栏（表情/图片）、文本编辑框和发送按钮
ChatInput::ChatInput(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("chatInputBar");

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 8, 16, 12);
    layout->setSpacing(6);

    // 工具栏：表情和图片按钮
    auto *toolbar = new QHBoxLayout;
    toolbar->setSpacing(2);
    toolbar->setContentsMargins(0, 0, 0, 0);

    m_emojiBtn = new QPushButton("😊");
    m_emojiBtn->setObjectName("chatToolBtn");
    m_emojiBtn->setToolTip("表情");
    toolbar->addWidget(m_emojiBtn);

    m_imageBtn = new QPushButton("🖼");
    m_imageBtn->setObjectName("chatToolBtn");
    m_imageBtn->setToolTip("发送图片");
    toolbar->addWidget(m_imageBtn);

    toolbar->addStretch();
    layout->addLayout(toolbar);

    // 文本输入框
    m_textEdit = new QTextEdit;
    m_textEdit->setObjectName("messageTextEdit");
    m_textEdit->setPlaceholderText("输入消息...");
    m_textEdit->setFixedHeight(68);
    layout->addWidget(m_textEdit);

    // 底部：发送按钮
    auto *sendBar = new QHBoxLayout;
    sendBar->setContentsMargins(0, 0, 0, 0);
    sendBar->addStretch();

    m_sendBtn = new QPushButton("发  送 (Enter)");
    m_sendBtn->setObjectName("sendBtn");
    m_sendBtn->setFixedHeight(34);
    m_sendBtn->setCursor(Qt::PointingHandCursor);
    sendBar->addWidget(m_sendBtn);
    layout->addLayout(sendBar);

    connect(m_sendBtn, &QPushButton::clicked, this, &ChatInput::onSendClicked);
    connect(m_emojiBtn, &QPushButton::clicked, this, &ChatInput::emojiToggled);
    connect(m_imageBtn, &QPushButton::clicked, this, [this]() {
        QString path = QFileDialog::getOpenFileName(this, "选择图片",
            QString(), "图片 (*.png *.jpg *.jpeg *.gif)");
        if (!path.isEmpty()) {
            emit sendImage(path);
        }
    });
}

// 发送文本：清空输入框
void ChatInput::onSendClicked() {
    QString text = m_textEdit->toPlainText().trimmed();
    if (!text.isEmpty()) {
        emit sendText(text);
        m_textEdit->clear();
    }
}

// 键盘事件：Enter 发送（Shift+Enter 换行）
void ChatInput::keyPressEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_Return && !(e->modifiers() & Qt::ShiftModifier)) {
        onSendClicked();
        return;
    }
    QWidget::keyPressEvent(e);
}
