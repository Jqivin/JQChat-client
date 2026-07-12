#include "contact_list.h"
#include "add_friend_dialog.h"
#include "viewmodels/contact_viewmodel.h"
#include "models/friend_model.h"
#include "utils/avatar_loader.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>

// 构造函数：布局标题栏、添加按钮和好友列表，连接信号
ContactList::ContactList(ContactViewModel *viewModel, QWidget *parent)
    : QWidget(parent)
    , m_viewModel(viewModel)
    , m_addDialog(nullptr)
{
    setStyleSheet("background: #2e2e2e;");

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // 标题栏
    auto *titleBar = new QWidget;
    titleBar->setStyleSheet("background: #2e2e2e; border-bottom: 1px solid #444;");
    auto *titleLayout = new QHBoxLayout(titleBar);
    titleLayout->setContentsMargins(20, 0, 20, 0);

    auto *title = new QLabel("联系人");
    title->setStyleSheet("color: white; font-size: 15px; font-weight: bold;");
    titleLayout->addWidget(title);
    titleLayout->addStretch();

    m_addBtn = new QPushButton("+ 添加好友");
    m_addBtn->setStyleSheet(
        "QPushButton { background: #07c160; color: white; border: none;"
        " padding: 4px 12px; border-radius: 4px; font-size: 13px; }"
        "QPushButton:hover { background: #06ad56; }");
    m_addBtn->setCursor(Qt::PointingHandCursor);
    titleLayout->addWidget(m_addBtn);
    layout->addWidget(titleBar);

    // 好友列表
    m_listWidget = new QListWidget;
    m_listWidget->setStyleSheet(
        "QListWidget { background: #2e2e2e; border: none; }"
        "QListWidget::item { padding: 10px 15px; border: none; color: white; }"
        "QListWidget::item:hover { background: #3a3a3a; }");
    m_listWidget->setIconSize(QSize(40, 40));
    m_listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    layout->addWidget(m_listWidget, 1);

    connect(m_addBtn, &QPushButton::clicked, this, &ContactList::onAddFriendClicked);
    connect(m_listWidget, &QListWidget::itemClicked, this, &ContactList::onFriendItemClicked);
    connect(m_viewModel->friendModel(), &FriendModel::friendListUpdated,
            this, &ContactList::onFriendListUpdated);
    connect(m_viewModel->friendModel(), &FriendModel::friendOnlineChanged,
            this, [this](quint64, bool) { onFriendListUpdated(); });
}

// 刷新好友列表：显示在线状态指示灯 + 头像 + 昵称
void ContactList::onFriendListUpdated() {
    m_friends = m_viewModel->friendModel()->friendList();
    m_listWidget->clear();

    for (const auto &f : m_friends) {
        // ---- 行容器 ----
        auto *row = new QFrame;
        row->setStyleSheet("background: transparent;");
        auto *rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(16, 10, 16, 10);
        rowLayout->setSpacing(14);
        rowLayout->setAlignment(Qt::AlignVCenter);

        // 头像（圆形）
        auto *avatar = new QLabel(f.nickname.left(1).toUpper());
        avatar->setFixedSize(44, 44);
        avatar->setAlignment(Qt::AlignCenter);
        QString avatarBg = f.online ? "#07c160" : "#555555";
        avatar->setStyleSheet(QString(
            "background: %1; border-radius: 22px; color: white;"
            " font-size: 18px; font-weight: bold;").arg(avatarBg));
        rowLayout->addWidget(avatar);

        // 异步加载真实头像
        if (!f.avatar_url.isEmpty()) {
            AvatarLoader::instance().load(f.avatar_url, [avatar](const QPixmap &pix) {
                if (!pix.isNull()) {
                    avatar->setPixmap(pix.scaled(44, 44, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                    avatar->setStyleSheet("border-radius: 22px;");
                    avatar->setText("");
                }
            });
        }

        // 昵称
        auto *nameLabel = new QLabel(f.nickname);
        nameLabel->setStyleSheet("color: white; font-size: 15px; background: transparent;");
        rowLayout->addWidget(nameLabel, 1);

        // 在线状态（小文字）
        auto *statusLabel = new QLabel(f.online ? "● 在线" : "● 离线");
        statusLabel->setStyleSheet(QString(
            "color: %1; font-size: 12px; background: transparent;")
            .arg(f.online ? "#07c160" : "#888"));
        rowLayout->addWidget(statusLabel);

        // ---- 添加到列表 ----
        auto *item = new QListWidgetItem;
        item->setData(Qt::UserRole, f.user_id);
        item->setSizeHint(QSize(0, 64));
        m_listWidget->addItem(item);
        m_listWidget->setItemWidget(item, row);
    }
}

void ContactList::onAddFriendClicked() {
    if (!m_addDialog) {
        m_addDialog = new AddFriendDialog(m_viewModel, this);
    }
    m_addDialog->exec();
}

void ContactList::onFriendItemClicked(QListWidgetItem *item) {
    quint64 uid = item->data(Qt::UserRole).toULongLong();
    emit friendSelected(uid);
}
