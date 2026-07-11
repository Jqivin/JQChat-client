#include "friend_request_list.h"
#include "network/api_manager.h"
#include <QHBoxLayout>
#include <QFrame>

FriendRequestList::FriendRequestList(QWidget *parent)
    : QWidget(parent)
    , m_api(ApiManager::instance())
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    auto *titleLabel = new QLabel("待处理好友请求");
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; padding: 10px;");
    layout->addWidget(titleLabel);

    m_listWidget = new QListWidget;
    m_listWidget->setStyleSheet(
        "QListWidget { border: none; background: white; }"
        "QListWidget::item { padding: 10px; border-bottom: 1px solid #eee; }");
    layout->addWidget(m_listWidget);
}

void FriendRequestList::addRequest(quint64 fromUid, const QString &fromName, const QString &remark) {
    // 去重
    for (const auto &r : m_requests) {
        if (r.fromUid == fromUid) return;
    }
    m_requests.append({fromUid, fromName, remark});
    refreshList();
    emit pendingCountChanged(m_requests.size());
}

void FriendRequestList::clearRequests() {
    m_requests.clear();
    refreshList();
    emit pendingCountChanged(0);
}

void FriendRequestList::refreshList() {
    m_listWidget->clear();
    for (int i = 0; i < m_requests.size(); ++i) {
        const auto &req = m_requests[i];

        auto *item = new QListWidgetItem;
        auto *widget = new QFrame;
        widget->setStyleSheet("QFrame { background: white; }");

        auto *hLayout = new QHBoxLayout(widget);

        // Avatar placeholder
        auto *avatar = new QLabel;
        avatar->setFixedSize(40, 40);
        avatar->setStyleSheet("background: #07c160; border-radius: 20px; color: white; font-weight: bold;");
        avatar->setAlignment(Qt::AlignCenter);
        avatar->setText(req.fromName.left(1).toUpper());
        hLayout->addWidget(avatar);

        auto *infoLayout = new QVBoxLayout;
        auto *nameLabel = new QLabel(req.fromName);
        nameLabel->setStyleSheet("font-size: 14px; font-weight: bold;");
        infoLayout->addWidget(nameLabel);

        if (!req.remark.isEmpty()) {
            auto *remarkLabel = new QLabel(QString("留言: %1").arg(req.remark));
            remarkLabel->setStyleSheet("color: #999; font-size: 12px;");
            infoLayout->addWidget(remarkLabel);
        }
        hLayout->addLayout(infoLayout);
        hLayout->addStretch();

        auto *acceptBtn = new QPushButton("同意");
        acceptBtn->setStyleSheet(
            "QPushButton { background: #07c160; color: white; border: none; "
            "padding: 6px 16px; border-radius: 4px; }"
            "QPushButton:hover { background: #06ad56; }");
        hLayout->addWidget(acceptBtn);

        auto *rejectBtn = new QPushButton("拒绝");
        rejectBtn->setStyleSheet(
            "QPushButton { background: #ddd; color: #333; border: none; "
            "padding: 6px 16px; border-radius: 4px; }"
            "QPushButton:hover { background: #ccc; }");
        hLayout->addWidget(rejectBtn);

        int index = i;
        connect(acceptBtn, &QPushButton::clicked, this, [this, index]() { onAcceptClicked(index); });
        connect(rejectBtn, &QPushButton::clicked, this, [this, index]() { onRejectClicked(index); });

        widget->setLayout(hLayout);
        item->setSizeHint(widget->sizeHint());
        m_listWidget->addItem(item);
        m_listWidget->setItemWidget(item, widget);
    }

    if (m_requests.isEmpty()) {
        auto *item = new QListWidgetItem;
        auto *emptyLabel = new QLabel("没有待处理的好友请求");
        emptyLabel->setAlignment(Qt::AlignCenter);
        emptyLabel->setStyleSheet("color: #999; padding: 40px;");
        item->setSizeHint(emptyLabel->sizeHint());
        m_listWidget->addItem(item);
        m_listWidget->setItemWidget(item, emptyLabel);
    }
}

void FriendRequestList::onAcceptClicked(int index) {
    if (index < 0 || index >= m_requests.size()) return;
    const auto &req = m_requests[index];
    m_api->handleFriendRequest(req.fromUid, "accept");
    m_requests.removeAt(index);
    refreshList();
    emit pendingCountChanged(m_requests.size());
}

void FriendRequestList::onRejectClicked(int index) {
    if (index < 0 || index >= m_requests.size()) return;
    const auto &req = m_requests[index];
    m_api->handleFriendRequest(req.fromUid, "reject");
    m_requests.removeAt(index);
    refreshList();
    emit pendingCountChanged(m_requests.size());
}
