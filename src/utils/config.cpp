#include "config.h"
#include <QStandardPaths>
#include <QDir>

// 获取单例实例（Meyer's Singleton）
AppConfig& AppConfig::instance() {
    static AppConfig config;
    return config;
}

AppConfig::AppConfig()
    : m_settings("JQChat", "JQChat")
{
}

// 读取配置中的服务器地址，默认 http://localhost:8080
QString AppConfig::serverUrl() const {
    return m_settings.value("server/url", "https://chat.jqivin.top").toString();
}

void AppConfig::setServerUrl(const QString &url) {
    m_settings.setValue("server/url", url);
}

// 从 HTTP 地址推导 WebSocket 地址（http→ws, https→wss）
QString AppConfig::wsUrl() const {
    QString httpUrl = serverUrl();
    return httpUrl.replace("http://", "ws://").replace("https://", "wss://") + "/ws";
}

void AppConfig::setWsUrl(const QString &url) {
    m_settings.setValue("server/ws_url", url);
}

// 获取数据库存储路径（AppData 目录下）
QString AppConfig::dbPath() const {
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    return dataDir + "/jqchat.db";
}
