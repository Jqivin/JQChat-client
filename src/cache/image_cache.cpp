#include "image_cache.h"

ImageCache* ImageCache::s_instance = nullptr;

// 获取单例实例
ImageCache* ImageCache::instance() {
    if (!s_instance) {
        s_instance = new ImageCache();
    }
    return s_instance;
}

// 构造函数：初始化缓存容量为 50 张图片
ImageCache::ImageCache(QObject *parent)
    : QObject(parent)
    , m_cache(50)
{
}

// 插入图片到缓存（QCache 自动管理内存）
void ImageCache::cacheImage(const QString &url, const QPixmap &pixmap) {
    m_cache.insert(url, new QPixmap(pixmap));
}

// 根据 URL 获取缓存的图片
QPixmap ImageCache::getImage(const QString &url) const {
    if (m_cache.contains(url)) {
        return *m_cache[url];
    }
    return QPixmap();
}

// 清空图片缓存
void ImageCache::clear() {
    m_cache.clear();
}
