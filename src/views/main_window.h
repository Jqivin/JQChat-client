#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QSplitter>
#include <QCloseEvent>

class ChatWindow;
class ContactList;
class FriendRequestList;
class PopupNotify;
class SettingDialog;
class TrayIcon;
class ApiManager;
class ChatViewModel;
class ContactViewModel;

// 主窗口：左侧会话/联系人面板 + 右侧聊天/联系人内容区
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // 登录成功后初始化（设置用户信息、连接 WebSocket、加载数据）
    void initAfterLogin();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    // 会话列表项被点击
    void onConversationSelected(QListWidgetItem *item);
    // 联系人列表选中好友
    void onFriendSelected(quint64 friendId);
    // 打开设置对话框
    void onShowSetting();
    // 退出登录
    void onLogout();
    // 点击头像触发更换
    void onAvatarClicked();

private:
    // 构建界面布局
    void setupUI();
    // 初始化系统托盘
    void setupTray();
    // 加载会话列表
    void loadConversations();
    // 显示消息通知
    void showNotification(const QString &title, const QString &content);
    // 切换到指定聊天
    void setCurrentChat(quint64 targetUid, const QString &name);

    QWidget *m_mainWidget;
    QSplitter *m_splitter;

    // Left panel
    QWidget *m_leftPanel;
    QLabel *m_selfAvatar;
    QLabel *m_selfName;
    QStackedWidget *m_leftStack;
    QListWidget *m_conversationList;
    ContactList *m_contactList;
    QPushButton *m_contactsBtn;
    QPushButton *m_settingsBtn;
    bool m_showingContacts = false;

    // Right panel
    QStackedWidget *m_rightStack;
    QWidget *m_emptyPage;
    ChatWindow *m_chatWindow;
    FriendRequestList *m_friendRequestList;

    // Dialogs
    SettingDialog *m_settingDialog;
    PopupNotify *m_popupNotify;

    TrayIcon *m_trayIcon;
    QPushButton *m_notifyBtn;
    int m_pendingCount = 0;

    ApiManager *m_api;
    ChatViewModel *m_chatVM;
    ContactViewModel *m_contactVM;
};

#endif // MAIN_WINDOW_H
