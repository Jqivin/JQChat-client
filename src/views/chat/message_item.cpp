#include "message_item.h"
#include "utils/avatar_loader.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPixmap>
#include <QMovie>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>

// 构造函数：根据消息类型和是否自己发送来构建气泡布局
MessageItem::MessageItem(const MessageData &msg, bool isSelf, const QString &avatarUrl, QWidget *parent)
    : QFrame(parent)
{
    setObjectName("messageItem");

    auto *outerLayout = new QHBoxLayout(this);
    outerLayout->setContentsMargins(10, 4, 10, 4);
    outerLayout->setSpacing(8);

    // 创建头像标签的 lambda
    auto makeAvatar = []() -> QLabel* {
        auto *avatar = new QLabel;
        avatar->setFixedSize(36, 36);
        avatar->setStyleSheet(
            "background: #ddd; border-radius: 18px;"
            "font-size: 14px; font-weight: bold; color: #fff;"
            "min-width: 36px; min-height: 36px;");
        avatar->setAlignment(Qt::AlignCenter);
        return avatar;
    };

    if (isSelf) {
        outerLayout->addStretch();
    } else {
        auto *avatar = makeAvatar();
        avatar->setText("?");
        avatar->setStyleSheet(
            "background: #87ceeb; border-radius: 18px;"
            "font-size: 14px; font-weight: bold; color: #fff;"
            "min-width: 36px; min-height: 36px;");
        // 异步加载头像
        if (!avatarUrl.isEmpty()) {
            AvatarLoader::instance().load(avatarUrl, [avatar](const QPixmap &pix) {
                if (!pix.isNull()) {
                    avatar->setPixmap(pix.scaled(36, 36, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                    avatar->setStyleSheet("border-radius: 18px; min-width: 36px; min-height: 36px;");
                    avatar->setText("");
                }
            });
        }
        outerLayout->addWidget(avatar);
    }

    // 内容列
    auto *contentColumn = new QVBoxLayout;
    contentColumn->setSpacing(2);

    m_contentLabel = new QLabel;
    m_contentLabel->setWordWrap(true);
    m_contentLabel->setMaximumWidth(380);
    m_contentLabel->setTextFormat(Qt::PlainText);

    if (msg.msg_type == MSG_TEXT) {
        setupText(msg, isSelf);
    } else if (msg.msg_type == MSG_IMAGE) {
        setupImage(msg, isSelf);
    } else if (msg.msg_type == MSG_EMOJI) {
        setupEmoji(msg, isSelf);
    }

    if (isSelf) {
        m_contentLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    }
    contentColumn->addWidget(m_contentLabel);

    // 时间标签
    m_timeLabel = new QLabel(msg.created_at.toString("HH:mm"));
    m_timeLabel->setObjectName("msgTimeLabel");
    if (isSelf) {
        m_timeLabel->setAlignment(Qt::AlignRight);
    }
    contentColumn->addWidget(m_timeLabel);

    if (isSelf) {
        outerLayout->addLayout(contentColumn);
        auto *avatar = makeAvatar();
        avatar->setText("我");
        avatar->setStyleSheet(
            "background: #1197fa; border-radius: 18px;"
            "font-size: 14px; font-weight: bold; color: #fff;"
            "min-width: 36px; min-height: 36px;");
        // 异步加载头像
        if (!avatarUrl.isEmpty()) {
            AvatarLoader::instance().load(avatarUrl, [avatar](const QPixmap &pix) {
                if (!pix.isNull()) {
                    avatar->setPixmap(pix.scaled(36, 36, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                    avatar->setStyleSheet("border-radius: 18px; min-width: 36px; min-height: 36px;");
                    avatar->setText("");
                }
            });
        }
        outerLayout->addWidget(avatar);
    } else {
        outerLayout->addLayout(contentColumn);
        outerLayout->addStretch();
    }
}

// 文字消息：设置蓝色（自己）/白色（对方）气泡背景
void MessageItem::setupText(const MessageData &msg, bool isSelf) {
    m_contentLabel->setText(msg.content);
    m_contentLabel->setStyleSheet(QString(
        "QLabel { background: %1; color: %2; padding: 10px 14px;"
        " border-radius: 8px; font-size: 14px; line-height: 1.5; }")
        .arg(isSelf ? "#cce5ff" : "#ffffff",
             isSelf ? "#000000" : "#333333"));
}

// 图片消息：加载缩略图显示（本地文件 / 服务器 URL），失败显示"[图片]"
void MessageItem::setupImage(const MessageData &msg, bool isSelf) {
    QString url = msg.file_url;

    // 本地文件：直接加载
    if (QFile::exists(url)) {
        QPixmap pix(url);
        if (!pix.isNull()) {
            pix = pix.scaled(240, 240, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            m_contentLabel->setPixmap(pix);
            m_contentLabel->setStyleSheet(QString(
                "QLabel { background: %1; padding: 4px; border-radius: 8px; }")
                .arg(isSelf ? "#cce5ff" : "#ffffff"));
            return;
        }
    }

    // 远程 URL：通过 AvatarLoader 下载
    if (!url.isEmpty()) {
        AvatarLoader::instance().load(url, [this, isSelf](const QPixmap &pix) {
            if (!pix.isNull()) {
                QPixmap scaled = pix.scaled(240, 240, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                m_contentLabel->setPixmap(scaled);
                m_contentLabel->setStyleSheet(QString(
                    "QLabel { background: %1; padding: 4px; border-radius: 8px; }")
                    .arg(isSelf ? "#cce5ff" : "#ffffff"));
            } else {
                m_contentLabel->setText("[图片]");
                m_contentLabel->setStyleSheet(QString(
                    "QLabel { background: %1; color: #666; padding: 10px 14px;"
                    " border-radius: 8px; font-size: 14px; }")
                    .arg(isSelf ? "#cce5ff" : "#ffffff"));
            }
        });
        return;
    }

    // 无 URL：占位
    m_contentLabel->setText("[图片]");
    m_contentLabel->setStyleSheet(QString(
        "QLabel { background: %1; color: #666; padding: 10px 14px;"
        " border-radius: 8px; font-size: 14px; }")
        .arg(isSelf ? "#cce5ff" : "#ffffff"));
}

// 表情消息：尝试加载表情图片，否则显示大号 Emoji 文字
void MessageItem::setupEmoji(const MessageData &msg, bool isSelf) {
    QPixmap pix(msg.file_url);
    if (!pix.isNull()) {
        pix = pix.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        m_contentLabel->setPixmap(pix);
    } else {
        m_contentLabel->setText(msg.content);
        m_contentLabel->setStyleSheet("font-size: 48px; background: transparent;");
    }
    m_contentLabel->setMaximumWidth(120);
}
