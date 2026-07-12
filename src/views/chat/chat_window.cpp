#include "chat_window.h"
#include "chat_input.h"
#include "emoji_picker.h"
#include "message_item.h"
#include "viewmodels/chat_viewmodel.h"
#include "models/message_model.h"
#include "network/api_manager.h"
#include "utils/avatar_loader.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QTimer>
#include <QLabel>

// 构造函数：布局标题栏、消息列表、表情面板和输入栏，连接信号
ChatWindow::ChatWindow(ChatViewModel *viewModel, QWidget *parent)
    : QWidget(parent)
    , m_viewModel(viewModel)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // 标题栏
    auto *titleBar = new QWidget;
    titleBar->setObjectName("chatTitleBar");
    titleBar->setFixedHeight(52);
    auto *titleLayout = new QHBoxLayout(titleBar);
    titleLayout->setContentsMargins(20, 0, 20, 0);

    m_titleLabel = new QLabel("选择会话开始聊天");
    m_titleLabel->setObjectName("chatTitleLabel");
    titleLayout->addWidget(m_titleLabel);
    titleLayout->addStretch();
    layout->addWidget(titleBar);

    // 消息滚动区域
    m_scrollArea = new QScrollArea;
    m_scrollArea->setObjectName("messageScrollArea");
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_messageContainer = new QWidget;
    m_messageContainer->setObjectName("messageContainer");
    m_messageLayout = new QVBoxLayout(m_messageContainer);
    m_messageLayout->setAlignment(Qt::AlignTop);
    m_messageLayout->setSpacing(8);
    m_messageLayout->setContentsMargins(16, 12, 16, 12);
    m_scrollArea->setWidget(m_messageContainer);
    layout->addWidget(m_scrollArea, 1);

    // 表情选择器（可折叠）
    m_emojiPicker = new EmojiPicker;
    m_emojiPicker->setObjectName("emojiPicker");
    m_emojiPicker->hide();
    layout->addWidget(m_emojiPicker);

    // 聊天输入栏
    m_chatInput = new ChatInput;
    layout->addWidget(m_chatInput);

    connect(m_chatInput, &ChatInput::sendText, this, &ChatWindow::onSendText);
    connect(m_chatInput, &ChatInput::sendImage, this, &ChatWindow::onSendImage);
    connect(m_chatInput, &ChatInput::emojiToggled, this, [this]() {
        m_emojiPicker->setVisible(!m_emojiPicker->isVisible());
    });
    connect(m_emojiPicker, &EmojiPicker::emojiSelected, this, &ChatWindow::onSendEmoji);
    connect(m_viewModel->messageModel(), &MessageModel::messagesUpdated,
            this, &ChatWindow::onMessagesUpdated);
    connect(m_viewModel, &ChatViewModel::typingReceived,
            this, &ChatWindow::onTypingReceived);
}

// 设置聊天目标并清空旧消息
void ChatWindow::setTarget(quint64 uid, const QString &name) {
    m_targetUid = uid;
    m_titleLabel->setText(name);
    clearMessages();
}

void ChatWindow::setUserAvatar(quint64 uid, const QString &url) {
    m_avatars[uid] = url;
}

// 根据消息创建 MessageItem 并添加到消息区域
void ChatWindow::addMessage(const MessageData &msg) {
    bool isSelf = msg.from_uid == ApiManager::instance()->selfInfo().user_id;
    quint64 avatarUid = isSelf ? msg.from_uid : msg.from_uid;
    // 优先取消息发送者的头像
    QString avatarUrl = m_avatars.value(msg.from_uid);
    auto *item = new MessageItem(msg, isSelf, avatarUrl);
    m_messageLayout->addWidget(item);
    scrollToBottom();
}

void ChatWindow::clearMessages() {
    QLayoutItem *child;
    while ((child = m_messageLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }
}

// 消息列表更新时刷新整个消息区域（目标匹配时）
void ChatWindow::onMessagesUpdated(quint64 targetUid) {
    if (targetUid != m_targetUid) return;
    clearMessages();

    auto msgs = m_viewModel->messageModel()->messages(targetUid);
    for (const auto &msg : msgs) {
        addMessage(msg);
    }
}

void ChatWindow::onSendText(const QString &text) {
    if (m_targetUid == 0) return;
    m_viewModel->sendTextMessage(m_targetUid, text);
}

void ChatWindow::onSendImage(const QString &path) {
    if (m_targetUid == 0) return;
    m_viewModel->sendImageMessage(m_targetUid, path);
}

void ChatWindow::onSendEmoji(const QString &code) {
    if (m_targetUid == 0) return;
    m_viewModel->sendEmojiMessage(m_targetUid, code);
    m_emojiPicker->hide();
}

// 显示"正在输入..."提示，3秒后自动消失
void ChatWindow::onTypingReceived(quint64 fromUid) {
    if (fromUid != m_targetUid) return;
    QString current = m_titleLabel->text();
    if (!current.contains("正在输入")) {
        m_titleLabel->setText(current + "  正在输入...");
    }
    QTimer::singleShot(3000, this, [this]() {
        QString t = m_titleLabel->text();
        m_titleLabel->setText(t.replace("  正在输入...", ""));
    });
}

// 延时滚动消息区域到底部
void ChatWindow::scrollToBottom() {
    QScrollBar *bar = m_scrollArea->verticalScrollBar();
    QTimer::singleShot(50, this, [bar]() {
        bar->setValue(bar->maximum());
    });
}
