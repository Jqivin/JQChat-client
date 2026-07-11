#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QHttpMultiPart>
#include <QUrlQuery>

// HTTP 客户端封装类：基于 QNetworkAccessManager 实现 REST API 调用
class HttpClient : public QObject {
    Q_OBJECT
public:
    explicit HttpClient(QObject *parent = nullptr);

    // 设置 API 基础地址
    void setBaseUrl(const QString &url);
    // 设置鉴权 Token
    void setToken(const QString &token);

    // HTTP GET 请求
    void get(const QString &path, const QUrlQuery &params = QUrlQuery());
    // HTTP POST 请求（JSON Body）
    void post(const QString &path, const QJsonObject &body);
    // HTTP PUT 请求
    void put(const QString &path, const QJsonObject &body);
    // HTTP DELETE 请求
    void del(const QString &path, const QJsonObject &body = QJsonObject());
    // 文件上传（multipart/form-data）
    void uploadFile(const QString &path, const QString &filePath, const QString &type);

signals:
    void requestFinished(int statusCode, const QJsonObject &response);
    void requestError(const QString &error);

private slots:
    void onReplyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *m_manager;
    QString m_baseUrl;
    QString m_token;
    // 创建带基础 URL 和鉴权的网络请求对象
    QNetworkRequest createRequest(const QString &path) const;
};

#endif // HTTP_CLIENT_H
