#include "credential_store.h"
#include <QByteArray>
#include <QCryptographicHash>
#include <QStandardPaths>
#include <QDir>

const char *CredentialStore::key() {
    // 固定密钥 + 机器名混淆
    return "JQChat@2024!Secure#Key";
}

QString CredentialStore::encrypt(const QString &plain) {
    QByteArray data = plain.toUtf8();
    QByteArray keyBytes(key());
    QByteArray result;
    result.resize(data.size());
    for (int i = 0; i < data.size(); ++i) {
        result[i] = data[i] ^ keyBytes[i % keyBytes.size()];
    }
    return QString::fromLatin1(result.toBase64());
}

QString CredentialStore::decrypt(const QString &cipher) {
    QByteArray data = QByteArray::fromBase64(cipher.toLatin1());
    QByteArray keyBytes(key());
    QByteArray result;
    result.resize(data.size());
    for (int i = 0; i < data.size(); ++i) {
        result[i] = data[i] ^ keyBytes[i % keyBytes.size()];
    }
    return QString::fromUtf8(result);
}

void CredentialStore::save(const QString &email, const QString &password, bool autoLogin) {
    QSettings settings("JQChat", "JQChat");
    settings.setValue("cred/email", encrypt(email));
    settings.setValue("cred/password", encrypt(password));
    settings.setValue("cred/autoLogin", autoLogin);
}

CredentialStore::Credentials CredentialStore::load() {
    Credentials cred;
    QSettings settings("JQChat", "JQChat");
    cred.email = decrypt(settings.value("cred/email").toString());
    cred.password = decrypt(settings.value("cred/password").toString());
    cred.autoLogin = settings.value("cred/autoLogin", false).toBool();
    return cred;
}

void CredentialStore::clear() {
    QSettings settings("JQChat", "JQChat");
    settings.remove("cred/email");
    settings.remove("cred/password");
    settings.remove("cred/autoLogin");
}
