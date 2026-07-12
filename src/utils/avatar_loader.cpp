#include "avatar_loader.h"
#include <QUrl>

AvatarLoader& AvatarLoader::instance() {
    static AvatarLoader inst;
    return inst;
}

AvatarLoader::AvatarLoader(QObject *parent)
    : QObject(parent)
    , m_manager(new QNetworkAccessManager(this))
{
}

void AvatarLoader::setBaseUrl(const QString &url) {
    m_baseUrl = url;
}

void AvatarLoader::load(const QString &avatarUrl, Callback callback) {
    // 空 URL：直接返回空图
    if (avatarUrl.isEmpty()) {
        callback(QPixmap());
        return;
    }

    // 命中缓存
    if (m_cache.contains(avatarUrl)) {
        callback(m_cache[avatarUrl]);
        return;
    }

    // 已有同名请求在进行中，加入等待队列
    if (m_pending.contains(avatarUrl)) {
        m_pending[avatarUrl].append(callback);
        return;
    }

    // 发起新请求
    m_pending[avatarUrl] = {callback};

    QString fullUrl = avatarUrl.startsWith("http")
        ? avatarUrl
        : m_baseUrl + avatarUrl;

    QNetworkReply *reply = m_manager->get(QNetworkRequest(QUrl(fullUrl)));
    connect(reply, &QNetworkReply::finished, this, [this, reply, avatarUrl]() {
        reply->deleteLater();

        QPixmap pix;
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            pix.loadFromData(data);
            if (!pix.isNull()) {
                m_cache[avatarUrl] = pix;
            }
        }

        // 通知所有等待的回调
        auto callbacks = m_pending.take(avatarUrl);
        for (const auto &cb : callbacks) {
            cb(pix);
        }
    });
}
