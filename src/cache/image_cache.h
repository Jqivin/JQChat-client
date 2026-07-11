#ifndef IMAGE_CACHE_H
#define IMAGE_CACHE_H

#include <QObject>
#include <QPixmap>
#include <QMap>
#include <QCache>

// 图片内存缓存类：基于 QCache 实现 LRU 淘汰
class ImageCache : public QObject {
    Q_OBJECT
public:
    // 获取单例实例
    static ImageCache* instance();

    // 缓存指定 URL 的图片
    void cacheImage(const QString &url, const QPixmap &pixmap);
    // 从缓存中获取图片，未命中返回空 QPixmap
    QPixmap getImage(const QString &url) const;
    // 清空所有缓存
    void clear();

private:
    explicit ImageCache(QObject *parent = nullptr);
    static ImageCache *s_instance;
    QCache<QString, QPixmap> m_cache;
};

#endif // IMAGE_CACHE_H
