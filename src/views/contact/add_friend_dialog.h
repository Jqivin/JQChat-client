#ifndef ADD_FRIEND_DIALOG_H
#define ADD_FRIEND_DIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include "network/proto.h"

class ContactViewModel;

// 添加好友对话框：按邮箱搜索用户并发送好友请求
class AddFriendDialog : public QDialog {
    Q_OBJECT
public:
    explicit AddFriendDialog(ContactViewModel *viewModel, QWidget *parent = nullptr);

private slots:
    // 点击搜索按钮
    void onSearchClicked();
    // 点击添加好友按钮
    void onAddClicked();
    // 搜索结果回调
    void onSearchResult(const FriendInfo &info, bool found);
    // 好友请求结果回调
    void onFriendAdded(bool success, const QString &msg);

private:
    ContactViewModel *m_viewModel;
    QLineEdit *m_emailEdit;
    QPushButton *m_searchBtn;
    QLabel *m_resultLabel;
    QPushButton *m_addBtn;
    FriendInfo m_searchedInfo;
};

#endif // ADD_FRIEND_DIALOG_H
