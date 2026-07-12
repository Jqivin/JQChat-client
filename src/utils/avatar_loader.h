#ifndef AVATAR_LOADER_H
#define AVATAR_LOADER_H

#include <QObject>
#include <QPixmap>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMap>
#include <functional>

// 头像加载器：从服务器异步下载头像并缓存，支持回调
class AvatarLoader : public QObject {
    Q_OBJECT
public:
    static AvatarLoader& instance();

    // 设置服务器基础 URL
    void setBaseUrl(const QString &url);

    // 异步加载头像（URL 为空则调用回调传入空 QPixmap）
    using Callback = std::function<void(const QPixmap &pixmap)>;
    void load(const QString &avatarUrl, Callback callback);

private:
    explicit AvatarLoader(QObject *parent = nullptr);
    QNetworkAccessManager *m_manager;
    QString m_baseUrl;
    QMap<QString, QList<Callback>> m_pending;  // URL → 等待的回调列表
    QMap<QString, QPixmap> m_cache;             // URL → 已缓存的头像
};

#endif // AVATAR_LOADER_H
