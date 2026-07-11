#ifndef FRIEND_REQUEST_LIST_H
#define FRIEND_REQUEST_LIST_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include "network/proto.h"

class ApiManager;

// 待处理好友请求列表：显示请求信息 + 接受/拒绝按钮
class FriendRequestList : public QWidget {
    Q_OBJECT
public:
    explicit FriendRequestList(QWidget *parent = nullptr);

    // 添加一条待处理请求
    void addRequest(quint64 fromUid, const QString &fromName, const QString &remark);
    // 获取待处理请求数量
    int pendingCount() const { return m_requests.size(); }
    // 清空所有请求
    void clearRequests();

signals:
    // 通知外部请求数量变化
    void pendingCountChanged(int count);

private slots:
    void onAcceptClicked(int index);
    void onRejectClicked(int index);

private:
    void refreshList();

    // 一条请求的数据
    struct PendingRequest {
        quint64 fromUid;
        QString fromName;
        QString remark;
    };

    QListWidget *m_listWidget;
    ApiManager *m_api;
    QList<PendingRequest> m_requests;
};

#endif // FRIEND_REQUEST_LIST_H
