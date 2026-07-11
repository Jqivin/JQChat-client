#ifndef CONTACT_LIST_H
#define CONTACT_LIST_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include "network/proto.h"

class ContactViewModel;
class AddFriendDialog;

// 联系人列表页面：标题栏 + 好友列表 + 添加按钮
class ContactList : public QWidget {
    Q_OBJECT
public:
    explicit ContactList(ContactViewModel *viewModel, QWidget *parent = nullptr);

signals:
    void friendSelected(quint64 friendId);

private slots:
    // 好友列表更新后刷新 UI
    void onFriendListUpdated();
    // 打开添加好友对话框
    void onAddFriendClicked();
    // 好友列表项被点击
    void onFriendItemClicked(QListWidgetItem *item);

private:
    ContactViewModel *m_viewModel;
    QListWidget *m_listWidget;
    QPushButton *m_addBtn;
    AddFriendDialog *m_addDialog;
    QList<FriendInfo> m_friends;
};

#endif // CONTACT_LIST_H
