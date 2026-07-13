#ifndef CREDENTIAL_STORE_H
#define CREDENTIAL_STORE_H

#include <QString>
#include <QSettings>

// 本地凭证存储：加密保存/读取 邮箱+密码+自动登录标记
class CredentialStore {
public:
    // 保存凭证（加密存储）
    static void save(const QString &email, const QString &password, bool autoLogin);

    // 读取凭证（解密），返回 {email, password, autoLogin}
    struct Credentials {
        QString email;
        QString password;
        bool autoLogin = false;
    };
    static Credentials load();

    // 清除所有保存的凭证
    static void clear();

private:
    // 简单 XOR 混淆 + Base64 编码
    static QString encrypt(const QString &plain);
    static QString decrypt(const QString &cipher);
    static const char *key();
};

#endif // CREDENTIAL_STORE_H
