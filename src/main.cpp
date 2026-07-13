#include "app/application.h"
#include "network/api_manager.h"
#include "viewmodels/setting_viewmodel.h"
#include "views/main_window.h"
#include "views/login/login_widget.h"
#include "viewmodels/login_viewmodel.h"
#include "utils/config.h"
#include "utils/logger.h"
#include "utils/avatar_loader.h"
#include "utils/credential_store.h"
#include "cache/database.h"
#include <QApplication>
#include <QStandardPaths>

// 程序入口：初始化配置、API管理器、本地数据库，显示登录窗口
int main(int argc, char *argv[]) {
    // 强制使用 Windows 原生 Schannel 做 TLS，不需要额外 OpenSSL DLL
    qputenv("QT_SSL_BACKEND", "schannel");

    QApplication app(argc, argv);
    app.setApplicationName("JQChat");
    app.setApplicationVersion("1.0.0");

    // 应用图标（任务栏、窗口左上角）
    app.setWindowIcon(QIcon(":/images/jqchat"));

    AppConfig &config = AppConfig::instance();

    // 初始化日志：日志文件保存在 AppData/JQChat/logs/
    QString logDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/logs";
    Logger::instance().init(logDir);

    AvatarLoader::instance().setBaseUrl(config.serverUrl());

    ApiManager *api = ApiManager::instance();
    api->setBaseUrl(config.serverUrl());
    api->setWsUrl(config.wsUrl());

    LocalDatabase::instance()->init(config.dbPath());

    MainWindow mainWindow;
    LoginWidget loginWidget;

    QObject::connect(&loginWidget, &LoginWidget::loginSuccess, [&]() {
        loginWidget.close();
        mainWindow.initAfterLogin();
        mainWindow.show();
    });

    // 自动登录：如果勾选了记住密码，后台直接调 API
    auto cred = CredentialStore::load();
    if (cred.autoLogin && !cred.email.isEmpty() && !cred.password.isEmpty()) {
        // 直接登录（不显示登录窗口）
        LoginViewModel tempVM;
        QObject::connect(&tempVM, &LoginViewModel::loginResult,
            [&](bool success, const QString &msg) {
                if (success) {
                    loginWidget.close();
                    mainWindow.initAfterLogin();
                    mainWindow.show();
                } else {
                    // 自动登录失败，显示登录窗口
                    CredentialStore::clear();
                    loginWidget.show();
                    loginWidget.raise();
                    loginWidget.activateWindow();
                }
            });
        tempVM.loginByPassword(cred.email, cred.password);
    } else {
        loginWidget.show();
        loginWidget.raise();
        loginWidget.activateWindow();
    }

    return app.exec();
}
