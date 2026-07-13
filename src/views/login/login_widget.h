#ifndef LOGIN_WIDGET_H
#define LOGIN_WIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QStackedWidget>
#include <QLabel>
#include <QCheckBox>
#include <QTimer>
#include <QPoint>

class LoginViewModel;

// 登录/注册窗口：无边框 660x1080，支持密码登录/验证码登录/注册三页面切换
class LoginWidget : public QWidget {
    Q_OBJECT
public:
    explicit LoginWidget(QWidget *parent = nullptr);

signals:
    void loginSuccess();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private slots:
    // 切换到密码登录页
    void onSwitchToPasswordLogin();
    // 切换到验证码登录页
    void onSwitchToCodeLogin();
    // 切换到注册页
    void onSwitchToRegister();
    // 发送验证码
    void onSendVerifyCode();
    // 执行登录
    void onLogin();
    // 执行注册
    void onRegister();
    // 验证码发送结果回调
    void onVerifyCodeResult(bool success, const QString &msg);
    // 登录结果回调
    void onLoginResult(bool success, const QString &msg);
    // 注册结果回调
    void onRegisterResult(bool success, const QString &msg);
    // 验证码倒计时
    void onCountdownTick();

private:
    // 构建界面布局
    void setupUI();
    // 开始验证码倒计时
    void startCountdown(int seconds);

    LoginViewModel *m_viewModel;

    QPushButton *m_minimizeBtn;
    QPushButton *m_closeBtn;
    QPoint m_dragPosition;

    QStackedWidget *m_stack;

    // Page 0: Login
    QWidget *m_loginPage;
    QLineEdit *m_loginEmail;
    QLineEdit *m_loginPassword;
    QCheckBox *m_autoLoginCb;
    QPushButton *m_loginBtn;
    QPushButton *m_switchToCodeLogin;
    QPushButton *m_switchToRegister;

    // Page 1: Code Login
    QWidget *m_codeLoginPage;
    QLineEdit *m_codeLoginEmail;
    QLineEdit *m_codeLoginCode;
    QPushButton *m_sendCodeBtn;
    QPushButton *m_codeLoginBtn;
    QPushButton *m_switchToPwdLogin;
    QLabel *m_countdownLabel;
    QTimer *m_countdownTimer;
    int m_countdown = 0;

    // Page 2: Register
    QWidget *m_registerPage;
    QLineEdit *m_regEmail;
    QLineEdit *m_regCode;
    QLineEdit *m_regPassword;
    QLineEdit *m_regNickname;
    QPushButton *m_regSendCodeBtn;
    QPushButton *m_regBtn;
    QPushButton *m_backToLogin;

    QLabel *m_statusLabel;
};

#endif // LOGIN_WIDGET_H
