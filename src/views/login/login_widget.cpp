#include "login_widget.h"
#include "viewmodels/login_viewmodel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QScreen>
#include <QGuiApplication>

// 构造函数：设置无边框窗口、固定大小、居中显示、连接信号
LoginWidget::LoginWidget(QWidget *parent)
    : QWidget(parent)
    , m_viewModel(new LoginViewModel(this))
    , m_countdownTimer(new QTimer(this))
{
    setWindowFlags(Qt::FramelessWindowHint);
    setFixedSize(444, 666);
    setAttribute(Qt::WA_TranslucentBackground, false);
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet("LoginWidget { background: #f5f5f5; }");

    setupUI();

    // 居中显示
    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
        QRect screenGeometry = screen->availableGeometry();
        int x = (screenGeometry.width() - width()) / 2;
        int y = (screenGeometry.height() - height()) / 2;
        move(x, y);
    }

    connect(m_viewModel, &LoginViewModel::verifyCodeResult,
            this, &LoginWidget::onVerifyCodeResult);
    connect(m_viewModel, &LoginViewModel::loginResult,
            this, &LoginWidget::onLoginResult);
    connect(m_viewModel, &LoginViewModel::registerResult,
            this, &LoginWidget::onRegisterResult);
    connect(m_countdownTimer, &QTimer::timeout, this, &LoginWidget::onCountdownTick);
}

// 构建完整 UI：标题栏（最小化/关闭）+ 内容区（Logo + 三页表单）
void LoginWidget::setupUI() {
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // --- Title bar (frameless window controls) ---
    auto *titleBarWidget = new QWidget;
    titleBarWidget->setFixedHeight(40);
    titleBarWidget->setStyleSheet("background: transparent;");

    auto *titleBarLayout = new QHBoxLayout(titleBarWidget);
    titleBarLayout->setContentsMargins(0, 0, 0, 0);
    titleBarLayout->setSpacing(0);
    titleBarLayout->addStretch();

    auto *minBtn = new QPushButton("—");
    minBtn->setFixedSize(40, 40);
    minBtn->setCursor(Qt::PointingHandCursor);
    minBtn->setStyleSheet(
        "QPushButton { border: none; background: transparent; font-size: 14px; color: #999; }"
        "QPushButton:hover { background: #e0e0e0; color: #333; }");
    titleBarLayout->addWidget(minBtn);

    auto *closeBtn = new QPushButton("✕");
    closeBtn->setFixedSize(40, 40);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(
        "QPushButton { border: none; background: transparent; font-size: 14px; color: #999; }"
        "QPushButton:hover { background: #e74c3c; color: white; }");
    titleBarLayout->addWidget(closeBtn);

    m_minimizeBtn = minBtn;
    m_closeBtn = closeBtn;

    connect(m_minimizeBtn, &QPushButton::clicked, this, &QWidget::showMinimized);
    connect(m_closeBtn, &QPushButton::clicked, this, &QWidget::close);

    mainLayout->addWidget(titleBarWidget);

    // --- Content area ---
    auto *contentWidget = new QWidget;
    contentWidget->setStyleSheet("background: #f5f5f5;");
    auto *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(50, 0, 50, 0);
    contentLayout->setSpacing(0);

    // App logo / title
    auto *appIcon = new QLabel("💬");
    appIcon->setAlignment(Qt::AlignCenter);
    appIcon->setStyleSheet("font-size: 56px; background: transparent;");
    contentLayout->addStretch(2);
    contentLayout->addWidget(appIcon);
    contentLayout->addSpacing(12);

    auto *title = new QLabel("JQChat");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size: 32px; font-weight: bold; color: #333; background: transparent;");
    contentLayout->addWidget(title);
    contentLayout->addSpacing(6);

    auto *subtitle = new QLabel("随时随地，保持联系");
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setStyleSheet("font-size: 14px; color: #999; background: transparent;");
    contentLayout->addWidget(subtitle);
    contentLayout->addSpacing(40);

    // Stacked form area
    m_stack = new QStackedWidget;
    m_stack->setStyleSheet("QStackedWidget { background: transparent; }");
    contentLayout->addWidget(m_stack);
    contentLayout->addSpacing(12);

    // Status label
    m_statusLabel = new QLabel;
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setWordWrap(true);
    m_statusLabel->setMinimumHeight(20);
    m_statusLabel->setStyleSheet("color: #e74c3c; font-size: 12px; background: transparent;");
    contentLayout->addWidget(m_statusLabel);
    contentLayout->addStretch(3);

    // ============ Page 0: Password Login ============
    m_loginPage = new QWidget;
    m_loginPage->setStyleSheet("background: transparent;");
    auto *loginLayout = new QVBoxLayout(m_loginPage);
    loginLayout->setContentsMargins(0, 0, 0, 0);
    loginLayout->setSpacing(16);

    m_loginEmail = new QLineEdit;
    m_loginEmail->setPlaceholderText("请输入邮箱");
    m_loginEmail->setFixedHeight(48);
    m_loginEmail->setStyleSheet(
        "QLineEdit { border: 1px solid #ddd; border-radius: 6px; padding: 0 16px;"
        " font-size: 15px; background: white; }"
        "QLineEdit:focus { border-color: #07c160; }");
    loginLayout->addWidget(m_loginEmail);

    m_loginPassword = new QLineEdit;
    m_loginPassword->setPlaceholderText("请输入密码");
    m_loginPassword->setEchoMode(QLineEdit::Password);
    m_loginPassword->setFixedHeight(48);
    m_loginPassword->setStyleSheet(
        "QLineEdit { border: 1px solid #ddd; border-radius: 6px; padding: 0 16px;"
        " font-size: 15px; background: white; }"
        "QLineEdit:focus { border-color: #07c160; }");
    loginLayout->addWidget(m_loginPassword);

    m_loginBtn = new QPushButton("登  录");
    m_loginBtn->setFixedHeight(48);
    m_loginBtn->setCursor(Qt::PointingHandCursor);
    m_loginBtn->setStyleSheet(
        "QPushButton { background: #07c160; color: white; border: none; border-radius: 6px;"
        " font-size: 16px; }"
        "QPushButton:hover { background: #06ad56; }"
        "QPushButton:pressed { background: #059a4c; }");
    loginLayout->addWidget(m_loginBtn);
    loginLayout->addSpacing(8);

    auto *loginLinks = new QHBoxLayout;
    loginLinks->setSpacing(0);
    m_switchToCodeLogin = new QPushButton("验证码登录");
    m_switchToCodeLogin->setCursor(Qt::PointingHandCursor);
    m_switchToCodeLogin->setStyleSheet(
        "QPushButton { border: none; background: transparent; color: #576b95; font-size: 13px; }"
        "QPushButton:hover { color: #07c160; }");
    m_switchToRegister = new QPushButton("注册账号");
    m_switchToRegister->setCursor(Qt::PointingHandCursor);
    m_switchToRegister->setStyleSheet(
        "QPushButton { border: none; background: transparent; color: #576b95; font-size: 13px; }"
        "QPushButton:hover { color: #07c160; }");
    loginLinks->addWidget(m_switchToCodeLogin);
    loginLinks->addStretch();
    loginLinks->addWidget(m_switchToRegister);
    loginLayout->addLayout(loginLinks);

    m_stack->addWidget(m_loginPage);

    // ============ Page 1: Code Login ============
    m_codeLoginPage = new QWidget;
    m_codeLoginPage->setStyleSheet("background: transparent;");
    auto *codeLayout = new QVBoxLayout(m_codeLoginPage);
    codeLayout->setContentsMargins(0, 0, 0, 0);
    codeLayout->setSpacing(16);

    m_codeLoginEmail = new QLineEdit;
    m_codeLoginEmail->setPlaceholderText("请输入邮箱");
    m_codeLoginEmail->setFixedHeight(48);
    m_codeLoginEmail->setStyleSheet(
        "QLineEdit { border: 1px solid #ddd; border-radius: 6px; padding: 0 16px;"
        " font-size: 15px; background: white; }"
        "QLineEdit:focus { border-color: #07c160; }");
    codeLayout->addWidget(m_codeLoginEmail);

    auto *codeRow = new QHBoxLayout;
    codeRow->setSpacing(8);
    m_codeLoginCode = new QLineEdit;
    m_codeLoginCode->setPlaceholderText("验证码");
    m_codeLoginCode->setFixedHeight(48);
    m_codeLoginCode->setStyleSheet(
        "QLineEdit { border: 1px solid #ddd; border-radius: 6px; padding: 0 16px;"
        " font-size: 15px; background: white; }"
        "QLineEdit:focus { border-color: #07c160; }");
    codeRow->addWidget(m_codeLoginCode, 1);

    m_sendCodeBtn = new QPushButton("获取验证码");
    m_sendCodeBtn->setFixedHeight(48);
    m_sendCodeBtn->setCursor(Qt::PointingHandCursor);
    m_sendCodeBtn->setStyleSheet(
        "QPushButton { background: white; color: #07c160; border: 1px solid #07c160;"
        " border-radius: 6px; font-size: 14px; }"
        "QPushButton:hover { background: #f0faf5; }"
        "QPushButton:disabled { color: #ccc; border-color: #ddd; }");
    codeRow->addWidget(m_sendCodeBtn);

    m_countdownLabel = new QLabel;
    m_countdownLabel->setStyleSheet("color: #999; font-size: 13px; background: transparent;");
    codeRow->addWidget(m_countdownLabel);
    codeLayout->addLayout(codeRow);

    m_codeLoginBtn = new QPushButton("登  录");
    m_codeLoginBtn->setFixedHeight(48);
    m_codeLoginBtn->setCursor(Qt::PointingHandCursor);
    m_codeLoginBtn->setStyleSheet(
        "QPushButton { background: #07c160; color: white; border: none; border-radius: 6px;"
        " font-size: 16px; }"
        "QPushButton:hover { background: #06ad56; }"
        "QPushButton:pressed { background: #059a4c; }");
    codeLayout->addWidget(m_codeLoginBtn);
    codeLayout->addSpacing(8);

    m_switchToPwdLogin = new QPushButton("密码登录");
    m_switchToPwdLogin->setCursor(Qt::PointingHandCursor);
    m_switchToPwdLogin->setStyleSheet(
        "QPushButton { border: none; background: transparent; color: #576b95; font-size: 13px; }"
        "QPushButton:hover { color: #07c160; }");
    codeLayout->addWidget(m_switchToPwdLogin);

    m_stack->addWidget(m_codeLoginPage);

    // ============ Page 2: Register ============
    m_registerPage = new QWidget;
    m_registerPage->setStyleSheet("background: transparent;");
    auto *regLayout = new QVBoxLayout(m_registerPage);
    regLayout->setContentsMargins(0, 0, 0, 0);
    regLayout->setSpacing(16);

    m_regEmail = new QLineEdit;
    m_regEmail->setPlaceholderText("请输入邮箱");
    m_regEmail->setFixedHeight(48);
    m_regEmail->setStyleSheet(
        "QLineEdit { border: 1px solid #ddd; border-radius: 6px; padding: 0 16px;"
        " font-size: 15px; background: white; }"
        "QLineEdit:focus { border-color: #07c160; }");
    regLayout->addWidget(m_regEmail);

    auto *regCodeRow = new QHBoxLayout;
    regCodeRow->setSpacing(8);
    m_regCode = new QLineEdit;
    m_regCode->setPlaceholderText("验证码");
    m_regCode->setFixedHeight(48);
    m_regCode->setStyleSheet(
        "QLineEdit { border: 1px solid #ddd; border-radius: 6px; padding: 0 16px;"
        " font-size: 15px; background: white; }"
        "QLineEdit:focus { border-color: #07c160; }");
    regCodeRow->addWidget(m_regCode, 1);
    m_regSendCodeBtn = new QPushButton("获取验证码");
    m_regSendCodeBtn->setFixedHeight(48);
    m_regSendCodeBtn->setCursor(Qt::PointingHandCursor);
    m_regSendCodeBtn->setStyleSheet(
        "QPushButton { background: white; color: #07c160; border: 1px solid #07c160;"
        " border-radius: 6px; font-size: 14px; }"
        "QPushButton:hover { background: #f0faf5; }"
        "QPushButton:disabled { color: #ccc; border-color: #ddd; }");
    regCodeRow->addWidget(m_regSendCodeBtn);
    regLayout->addLayout(regCodeRow);

    m_regPassword = new QLineEdit;
    m_regPassword->setPlaceholderText("设置密码 (6-20位)");
    m_regPassword->setEchoMode(QLineEdit::Password);
    m_regPassword->setFixedHeight(48);
    m_regPassword->setStyleSheet(
        "QLineEdit { border: 1px solid #ddd; border-radius: 6px; padding: 0 16px;"
        " font-size: 15px; background: white; }"
        "QLineEdit:focus { border-color: #07c160; }");
    regLayout->addWidget(m_regPassword);

    m_regNickname = new QLineEdit;
    m_regNickname->setPlaceholderText("设置昵称 (选填)");
    m_regNickname->setFixedHeight(48);
    m_regNickname->setStyleSheet(
        "QLineEdit { border: 1px solid #ddd; border-radius: 6px; padding: 0 16px;"
        " font-size: 15px; background: white; }"
        "QLineEdit:focus { border-color: #07c160; }");
    regLayout->addWidget(m_regNickname);

    m_regBtn = new QPushButton("注  册");
    m_regBtn->setFixedHeight(48);
    m_regBtn->setCursor(Qt::PointingHandCursor);
    m_regBtn->setStyleSheet(
        "QPushButton { background: #07c160; color: white; border: none; border-radius: 6px;"
        " font-size: 16px; }"
        "QPushButton:hover { background: #06ad56; }"
        "QPushButton:pressed { background: #059a4c; }");
    regLayout->addWidget(m_regBtn);
    regLayout->addSpacing(8);

    m_backToLogin = new QPushButton("← 返回登录");
    m_backToLogin->setCursor(Qt::PointingHandCursor);
    m_backToLogin->setStyleSheet(
        "QPushButton { border: none; background: transparent; color: #576b95; font-size: 13px; }"
        "QPushButton:hover { color: #07c160; }");
    regLayout->addWidget(m_backToLogin);

    m_stack->addWidget(m_registerPage);

    // ============ Connections ============
    connect(m_switchToCodeLogin, &QPushButton::clicked, this, &LoginWidget::onSwitchToCodeLogin);
    connect(m_switchToRegister, &QPushButton::clicked, this, &LoginWidget::onSwitchToRegister);
    connect(m_switchToPwdLogin, &QPushButton::clicked, this, &LoginWidget::onSwitchToPasswordLogin);
    connect(m_backToLogin, &QPushButton::clicked, this, &LoginWidget::onSwitchToPasswordLogin);
    connect(m_sendCodeBtn, &QPushButton::clicked, this, &LoginWidget::onSendVerifyCode);
    connect(m_regSendCodeBtn, &QPushButton::clicked, this, &LoginWidget::onSendVerifyCode);
    connect(m_loginBtn, &QPushButton::clicked, this, &LoginWidget::onLogin);
    connect(m_codeLoginBtn, &QPushButton::clicked, this, &LoginWidget::onLogin);
    connect(m_regBtn, &QPushButton::clicked, this, &LoginWidget::onRegister);

    m_stack->setCurrentIndex(0);
    mainLayout->addWidget(contentWidget);
}

// 鼠标按下：记录拖动起始位置（实现窗口拖拽）
void LoginWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

// 鼠标移动：拖动窗口
void LoginWidget::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPos() - m_dragPosition);
        event->accept();
    }
}

// 切换到密码登录页（索引 0）
void LoginWidget::onSwitchToPasswordLogin() {
    m_statusLabel->clear();
    m_stack->setCurrentIndex(0);
}

// 切换到验证码登录页（索引 1）
void LoginWidget::onSwitchToCodeLogin() {
    m_statusLabel->clear();
    m_stack->setCurrentIndex(1);
}

// 切换到注册页（索引 2）
void LoginWidget::onSwitchToRegister() {
    m_statusLabel->clear();
    m_stack->setCurrentIndex(2);
}

// 发送验证码：根据当前页面获取邮箱和场景
void LoginWidget::onSendVerifyCode() {
    m_statusLabel->clear();
    QString email;
    QString scene;
    int currentPage = m_stack->currentIndex();

    if (currentPage == 1) {
        email = m_codeLoginEmail->text().trimmed();
        scene = "login";
    } else if (currentPage == 2) {
        email = m_regEmail->text().trimmed();
        scene = "register";
    } else {
        return;
    }

    if (email.isEmpty()) {
        m_statusLabel->setText("请输入邮箱地址");
        return;
    }

    m_viewModel->sendVerifyCode(email, scene);
}

// 执行登录：根据当前页面选择密码登录或验证码登录
void LoginWidget::onLogin() {
    m_statusLabel->clear();
    int currentPage = m_stack->currentIndex();
    if (currentPage == 0) {
        QString email = m_loginEmail->text().trimmed();
        QString password = m_loginPassword->text();
        if (email.isEmpty() || password.isEmpty()) {
            m_statusLabel->setText("请填写邮箱和密码");
            return;
        }
        m_viewModel->loginByPassword(email, password);
    } else if (currentPage == 1) {
        QString email = m_codeLoginEmail->text().trimmed();
        QString code = m_codeLoginCode->text().trimmed();
        if (email.isEmpty() || code.isEmpty()) {
            m_statusLabel->setText("请填写邮箱和验证码");
            return;
        }
        m_viewModel->loginByVerifyCode(email, code);
    }
}

// 执行注册：校验表单后调用 ViewModel
void LoginWidget::onRegister() {
    m_statusLabel->clear();
    QString email = m_regEmail->text().trimmed();
    QString code = m_regCode->text().trimmed();
    QString password = m_regPassword->text();
    QString nickname = m_regNickname->text().trimmed();

    if (email.isEmpty() || code.isEmpty() || password.isEmpty()) {
        m_statusLabel->setText("请填写完整信息");
        return;
    }

    m_viewModel->registerUser(email, password, code, nickname);
}

// 验证码发送结果：成功则开始 60s 倒计时
void LoginWidget::onVerifyCodeResult(bool success, const QString &msg) {
    if (success) {
        m_statusLabel->setStyleSheet("color: #07c160; font-size: 12px; background: transparent;");
        m_statusLabel->setText("验证码已发送，请查收邮箱");
        startCountdown(60);
    } else {
        m_statusLabel->setStyleSheet("color: #e74c3c; font-size: 12px; background: transparent;");
        m_statusLabel->setText(msg);
    }
}

// 登录结果：成功则发射 loginSuccess 信号
void LoginWidget::onLoginResult(bool success, const QString &msg) {
    if (success) {
        m_statusLabel->setStyleSheet("color: #07c160; font-size: 12px; background: transparent;");
        m_statusLabel->setText("登录成功");
        emit loginSuccess();
    } else {
        m_statusLabel->setStyleSheet("color: #e74c3c; font-size: 12px; background: transparent;");
        m_statusLabel->setText(msg);
    }
}

// 注册结果：成功后自动进入主界面
void LoginWidget::onRegisterResult(bool success, const QString &msg) {
    if (success) {
        m_statusLabel->setStyleSheet("color: #07c160; font-size: 12px; background: transparent;");
        m_statusLabel->setText("注册成功，正在进入...");
        emit loginSuccess();
    } else {
        m_statusLabel->setStyleSheet("color: #e74c3c; font-size: 12px; background: transparent;");
        m_statusLabel->setText(msg);
    }
}

// 开始验证码按钮倒计时
void LoginWidget::startCountdown(int seconds) {
    m_countdown = seconds;
    m_sendCodeBtn->setEnabled(false);
    m_regSendCodeBtn->setEnabled(false);
    m_countdownLabel->setText(QString::number(seconds) + "s");
    m_countdownTimer->start(1000);
}

// 倒计时每秒更新，结束后恢复按钮
void LoginWidget::onCountdownTick() {
    m_countdown--;
    if (m_countdown <= 0) {
        m_countdownTimer->stop();
        m_sendCodeBtn->setEnabled(true);
        m_regSendCodeBtn->setEnabled(true);
        m_countdownLabel->clear();
    } else {
        m_countdownLabel->setText(QString::number(m_countdown) + "s");
    }
}
