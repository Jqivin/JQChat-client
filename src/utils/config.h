#ifndef CONFIG_H
#define CONFIG_H

#include <QString>
#include <QSettings>

// 应用配置类：基于 QSettings 读写永久设置
class AppConfig {
public:
    // 获取单例实例
    static AppConfig& instance();

    // 服务器 HTTP 地址
    QString serverUrl() const;
    void setServerUrl(const QString &url);

    // WebSocket 地址（自动从 HTTP 地址转换）
    QString wsUrl() const;
    void setWsUrl(const QString &url);

    // 本地 SQLite 数据库路径
    QString dbPath() const;

private:
    AppConfig();
    QSettings m_settings;
};

#endif // CONFIG_H
