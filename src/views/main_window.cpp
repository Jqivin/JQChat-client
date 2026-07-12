#include "main_window.h"
#include "chat/chat_window.h"
#include "contact/contact_list.h"
#include "contact/friend_request_list.h"
#include "setting/setting_dialog.h"
#include "widgets/tray_icon.h"
#include "widgets/popup_notify.h"
#include "widgets/crop_avatar_dialog.h"
#include "widgets/title_bar.h"
#include "network/api_manager.h"
#include "viewmodels/chat_viewmodel.h"
#include "viewmodels/contact_viewmodel.h"
#include "models/message_model.h"
#include "models/friend_model.h"
#include "utils/avatar_loader.h"
#include "cache/database.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QFileDialog>
#include <QApplication>
#include <QMouseEvent>
#include <QStandardPaths>
#include <QFileDialog>

// 构造函数：构建 UI、托盘图标，连接 WebSocket 状态信号和消息通知信号
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_api(ApiManager::instance())
    , m_chatVM(new ChatViewModel(this))
    , m_contactVM(new ContactViewModel(this))
    , m_settingDialog(nullptr)
    , m_friendRequestList(nullptr)
    , m_popupNotify(new PopupNotify(this))
    , m_notifyBtn(nullptr)
    , m_pendingCount(0)
{
    // 无边框窗口
    setWindowFlags(Qt::FramelessWindowHint);
    // 保留窗口阴影（Windows）
    setAttribute(Qt::WA_TranslucentBackground, false);

    setupUI();
    setupTray();

    connect(m_api, &ApiManager::wsConnected, this, [this]() {
        setWindowTitle("JQChat - 已连接");
    });
    connect(m_api, &ApiManager::wsDisconnected, this, [this]() {
        setWindowTitle("JQChat - 已断开");
    });

    connect(m_chatVM->messageModel(), &MessageModel::newMessageReceived,
            this, [this](const MessageData &msg) {
        if (msg.from_uid != m_api->selfInfo().user_id) {
            m_popupNotify->showNotification("新消息", msg.content);
            showNotification("新消息", msg.content);
        }
    });

    connect(m_api, &ApiManager::friendRequestReceived,
            this, [this](quint64 fromUid, const QString &fromName, const QString &remark) {
        // 弹窗通知
        m_popupNotify->showNotification("好友申请", QString("%1 请求添加你为好友").arg(fromName));
        showNotification("好友申请", QString("%1 请求添加你为好友").arg(fromName));

        // 添加到待处理列表
        if (!m_friendRequestList) {
            m_friendRequestList = new FriendRequestList(this);
            m_rightStack->addWidget(m_friendRequestList);
            connect(m_friendRequestList, &FriendRequestList::pendingCountChanged,
                    this, [this](int count) {
                m_pendingCount = count;
                if (m_notifyBtn) {
                    if (count > 0) {
                        m_notifyBtn->setText(QString("通知 (%1)").arg(count));
                        m_notifyBtn->setStyleSheet(
                            "QPushButton { background: #fa5151; color: white; border: none; padding: 8px; border-radius: 4px; }"
                            "QPushButton:hover { background: #dd4040; }");
                    } else {
                        m_notifyBtn->setText("通知");
                        m_notifyBtn->setStyleSheet(
                            "QPushButton { background: #3a3a3a; color: white; border: none; padding: 8px; border-radius: 4px; }"
                            "QPushButton:hover { background: #4a4a4a; }");
                    }
                }
            });
            connect(m_api, &ApiManager::friendRequestHandled,
                    this, [this](bool) {
                m_contactVM->loadFriendList();
            });
        }
        m_friendRequestList->addRequest(fromUid, fromName, remark);
    });

    connect(m_popupNotify, &PopupNotify::showWindow, this, [this]() {
        show();
        raise();
        activateWindow();
    });

    // 头像上传成功 → 刷新显示
    connect(m_api, &ApiManager::userInfoReady,
            this, [this](const UserInfo &info) {
        AvatarLoader::instance().load(info.avatar_url, [this](const QPixmap &pix) {
            if (!pix.isNull()) {
                m_selfAvatar->setPixmap(pix.scaled(40, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                m_selfAvatar->setStyleSheet("border-radius: 20px;");
            }
        });
    });
}

MainWindow::~MainWindow() {}

// 构建主界面：左侧（会话列表/联系人列表）+ 右侧（聊天/通知）
void MainWindow::setupUI() {
    m_mainWidget = new QWidget;
    setCentralWidget(m_mainWidget);

    auto *outerLayout = new QVBoxLayout(m_mainWidget);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);

    // 自定义标题栏
    auto *titleBar = new TitleBar(this);
    outerLayout->addWidget(titleBar);

    auto *mainLayout = new QHBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->addLayout(mainLayout, 1);

    m_splitter = new QSplitter(Qt::Horizontal);

    // ===== Left panel =====
    m_leftPanel = new QWidget;
    m_leftPanel->setStyleSheet("background: #2e2e2e;");
    auto *leftLayout = new QVBoxLayout(m_leftPanel);
    leftLayout->setContentsMargins(0, 10, 0, 10);
    leftLayout->setSpacing(10);

    // Self info
    auto *selfInfoLayout = new QHBoxLayout;
    selfInfoLayout->setContentsMargins(15, 5, 15, 5);
    m_selfAvatar = new QLabel;
    m_selfAvatar->setFixedSize(40, 40);
    m_selfAvatar->setStyleSheet("background: #07c160; border-radius: 20px;");
    m_selfAvatar->setCursor(Qt::PointingHandCursor);
    m_selfAvatar->installEventFilter(this);
    selfInfoLayout->addWidget(m_selfAvatar);

    m_selfName = new QLabel;
    m_selfName->setStyleSheet("color: white; font-size: 14px; font-weight: bold;");
    selfInfoLayout->addWidget(m_selfName);
    selfInfoLayout->addStretch();
    leftLayout->addLayout(selfInfoLayout);

    // Left stack: 会话列表 / 联系人列表
    m_leftStack = new QStackedWidget;

    m_conversationList = new QListWidget;
    m_conversationList->setMinimumWidth(250);
    m_conversationList->setStyleSheet(
        "QListWidget { background: #2e2e2e; border: none; color: white; }"
        "QListWidget::item { padding: 10px 15px; border: none; }"
        "QListWidget::item:hover { background: #3a3a3a; }"
        "QListWidget::item:selected { background: #07c160; }");
    m_leftStack->addWidget(m_conversationList);  // index 0

    m_contactList = new ContactList(m_contactVM);
    m_leftStack->addWidget(m_contactList);       // index 1

    leftLayout->addWidget(m_leftStack, 1);

    // Bottom buttons
    auto *btnLayout = new QHBoxLayout;
    btnLayout->setContentsMargins(15, 5, 15, 5);

    auto btnStyle = []() {
        return "QPushButton { background: #3a3a3a; color: white; border: none;"
               " padding: 8px; border-radius: 4px; }"
               "QPushButton:hover { background: #4a4a4a; }";
    };

    m_notifyBtn = new QPushButton("通知");
    m_notifyBtn->setStyleSheet(btnStyle());
    m_contactsBtn = new QPushButton("联系人");
    m_contactsBtn->setStyleSheet(btnStyle());
    m_settingsBtn = new QPushButton("设置");
    m_settingsBtn->setStyleSheet(btnStyle());
    btnLayout->addWidget(m_notifyBtn);
    btnLayout->addWidget(m_contactsBtn);
    btnLayout->addWidget(m_settingsBtn);
    leftLayout->addLayout(btnLayout);

    m_splitter->addWidget(m_leftPanel);

    // ===== Right panel =====
    m_rightStack = new QStackedWidget;

    m_emptyPage = new QWidget;
    m_emptyPage->setStyleSheet("background: #f5f5f5;");
    auto *emptyLayout = new QVBoxLayout(m_emptyPage);
    auto *emptyLabel = new QLabel("选择一个会话开始聊天");
    emptyLabel->setAlignment(Qt::AlignCenter);
    emptyLabel->setStyleSheet("color: #999; font-size: 16px; background: transparent;");
    emptyLayout->addWidget(emptyLabel);
    m_rightStack->addWidget(m_emptyPage);     // index 0

    m_chatWindow = new ChatWindow(m_chatVM);
    m_rightStack->addWidget(m_chatWindow);    // index 1

    m_splitter->addWidget(m_rightStack);
    m_splitter->setStretchFactor(0, 1);
    m_splitter->setStretchFactor(1, 3);

    mainLayout->addWidget(m_splitter);

    // ===== Connections =====
    connect(m_conversationList, &QListWidget::itemClicked,
            this, &MainWindow::onConversationSelected);

    // 联系人按钮：切换左侧面板并高亮按钮
    connect(m_contactsBtn, &QPushButton::clicked, this, [this]() {
        m_showingContacts = !m_showingContacts;
        if (m_showingContacts) {
            m_leftStack->setCurrentIndex(1);
            m_contactsBtn->setStyleSheet(
                "QPushButton { background: #07c160; color: white; border: none;"
                " padding: 8px; border-radius: 4px; }");
        } else {
            m_leftStack->setCurrentIndex(0);
            m_contactsBtn->setStyleSheet(
                "QPushButton { background: #3a3a3a; color: white; border: none;"
                " padding: 8px; border-radius: 4px; }"
                "QPushButton:hover { background: #4a4a4a; }");
        }
    });

    // 通知按钮
    connect(m_notifyBtn, &QPushButton::clicked, this, [this]() {
        if (!m_friendRequestList) {
            m_friendRequestList = new FriendRequestList(this);
            m_rightStack->addWidget(m_friendRequestList);
            connect(m_friendRequestList, &FriendRequestList::pendingCountChanged,
                    this, [this](int count) {
                m_pendingCount = count;
                if (m_notifyBtn) {
                    if (count > 0) {
                        m_notifyBtn->setText(QString("通知 (%1)").arg(count));
                        m_notifyBtn->setStyleSheet(
                            "QPushButton { background: #fa5151; color: white; border: none;"
                            " padding: 8px; border-radius: 4px; }"
                            "QPushButton:hover { background: #dd4040; }");
                    } else {
                        m_notifyBtn->setText("通知");
                        m_notifyBtn->setStyleSheet(
                            "QPushButton { background: #3a3a3a; color: white; border: none;"
                            " padding: 8px; border-radius: 4px; }"
                            "QPushButton:hover { background: #4a4a4a; }");
                    }
                }
            });
            connect(m_api, &ApiManager::friendRequestHandled,
                    this, [this](bool) { m_contactVM->loadFriendList(); });
        }
        m_rightStack->setCurrentWidget(m_friendRequestList);
    });

    connect(m_settingsBtn, &QPushButton::clicked, this, &MainWindow::onShowSetting);
    connect(m_contactList, &ContactList::friendSelected, this, &MainWindow::onFriendSelected);

    resize(1000, 700);
    setMinimumSize(800, 600);
    setWindowTitle("JQChat");
    setWindowIcon(QIcon(":/images/jqchat"));
}

// 创建系统托盘图标及右键菜单
void MainWindow::setupTray() {
    m_trayIcon = new TrayIcon(this);
    connect(m_trayIcon, &TrayIcon::showWindow, this, [this]() {
        if (m_loggedIn) {
            show();
            raise();
            activateWindow();
            m_trayIcon->stopBlink();
        }
    });
}

// 登录后初始化：设置昵称、连接 WebSocket、加载会话和好友列表
void MainWindow::initAfterLogin() {
    m_loggedIn = true;
    m_chatVM->messageModel()->setSelfUid(m_api->selfInfo().user_id);
    m_selfName->setText(m_api->selfInfo().nickname);
    m_api->connectWebSocket();

    // 设置自己头像（左上角 + 聊天消息）
    const QString &selfAvatar = m_api->selfInfo().avatar_url;
    m_chatWindow->setUserAvatar(m_api->selfInfo().user_id, selfAvatar);

    if (!selfAvatar.isEmpty()) {
        AvatarLoader::instance().load(selfAvatar, [this](const QPixmap &pix) {
            if (!pix.isNull()) {
                m_selfAvatar->setPixmap(pix.scaled(40, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                m_selfAvatar->setStyleSheet("border-radius: 20px;");
            }
        });
    }

    // 好友列表加载完 → 设置好友头像
    connect(m_contactVM, &ContactViewModel::friendListLoaded,
            this, [this](bool) {
        for (const auto &f : m_contactVM->friendModel()->friendList()) {
            m_chatWindow->setUserAvatar(f.user_id, f.avatar_url);
        }
    });

    loadConversations();
    m_contactVM->loadFriendList();
}

// 从 MessageModel 获取会话列表并填充到左侧列表控件
void MainWindow::loadConversations() {
    m_conversationList->clear();
    auto convs = m_chatVM->messageModel()->conversations();
    for (const auto &c : convs) {
        auto *item = new QListWidgetItem(
            QString("用户 %1").arg(c.target_uid));
        item->setData(Qt::UserRole, c.target_uid);
        m_conversationList->addItem(item);
    }
}

void MainWindow::onConversationSelected(QListWidgetItem *item) {
    quint64 uid = item->data(Qt::UserRole).toULongLong();
    QString name = item->text();
    setCurrentChat(uid, name);
}

void MainWindow::onFriendSelected(quint64 friendId) {
    auto fi = m_contactVM->friendModel()->findFriend(friendId);
    setCurrentChat(friendId, fi.nickname);
}

// 切换到指定聊天：设置目标、加载历史、标记已读、显示聊天窗口
void MainWindow::setCurrentChat(quint64 targetUid, const QString &name) {
    m_chatWindow->setTarget(targetUid, name);
    m_chatVM->loadHistoryMessages(targetUid);
    m_chatVM->markAsRead(targetUid);
    m_rightStack->setCurrentWidget(m_chatWindow);
}

// 显示设置对话框（懒加载）
void MainWindow::onShowSetting() {
    if (!m_settingDialog) {
        m_settingDialog = new SettingDialog(this);
    }
    m_settingDialog->exec();
}

// 退出登录：断开 WebSocket 并退出应用
void MainWindow::onLogout() {
    m_api->disconnectWebSocket();
    QApplication::quit();
}

// 点击头像：选择图片 → 裁剪 → 上传
void MainWindow::onAvatarClicked() {
    QString path = QFileDialog::getOpenFileName(this, "选择头像图片",
        QString(), "图片 (*.png *.jpg *.jpeg *.gif *.bmp)");
    if (path.isEmpty()) return;

    // 打开裁剪弹窗
    CropAvatarDialog dialog(path, this);
    if (dialog.exec() != QDialog::Accepted) return;

    // 保存裁剪结果到临时文件
    QPixmap cropped = dialog.croppedPixmap();
    if (cropped.isNull()) return;

    QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation)
                       + "/jqchat_avatar.png";
    cropped.save(tempPath, "PNG");

    m_api->uploadAvatar(tempPath);
}

// 新消息通知：托盘闪烁 + 托盘气泡提示
void MainWindow::showNotification(const QString &title, const QString &content) {
    m_trayIcon->startBlink();
    if (isMinimized() || !isVisible()) {
        m_trayIcon->showMessage(title, content);
    }
}

// 窗口激活时停止托盘闪烁
void MainWindow::changeEvent(QEvent *event) {
    if (event->type() == QEvent::ActivationChange && isActiveWindow()) {
        m_trayIcon->stopBlink();
    }
    QMainWindow::changeEvent(event);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
    if (obj == m_selfAvatar && event->type() == QEvent::MouseButtonPress) {
        onAvatarClicked();
        return true;
    }
    return QMainWindow::eventFilter(obj, event);
}

// 关闭事件：断开 WebSocket 并退出应用
void MainWindow::closeEvent(QCloseEvent *event) {
    m_api->disconnectWebSocket();
    QApplication::quit();
    event->accept();
}
