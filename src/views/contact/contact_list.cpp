#include "contact_list.h"
#include "add_friend_dialog.h"
#include "viewmodels/contact_viewmodel.h"
#include "models/friend_model.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

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
}

// 刷新好友列表：显示在线状态图标、昵称和备注
void ContactList::onFriendListUpdated() {
    m_friends = m_viewModel->friendModel()->friendList();
    m_listWidget->clear();

    for (const auto &f : m_friends) {
        // 在线绿点，离线灰点
        QString dotColor = f.online ? "#07c160" : "#999999";
        QString display = QString(
            "<span style='color:%1; font-size:18px;'>&#9679;</span>"
            "&nbsp;&nbsp;<span style='color:white; font-size:14px;'>%2</span>")
            .arg(dotColor, f.nickname);

        if (!f.remark.isEmpty()) {
            display += QString(
                "&nbsp;<span style='color:#999; font-size:12px;'>(%1)</span>")
                .arg(f.remark);
        }

        auto *item = new QListWidgetItem;
        auto *label = new QLabel(display);
        label->setStyleSheet("background: transparent;");
        m_listWidget->addItem(item);
        m_listWidget->setItemWidget(item, label);

        item->setData(Qt::UserRole, f.user_id);
        item->setSizeHint(QSize(0, 52));
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
