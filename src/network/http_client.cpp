#include "http_client.h"
#include "utils/logger.h"
#include <QUrlQuery>
#include <QJsonDocument>
#include <QFile>
#include <QFileInfo>
#include <QMimeDatabase>

// 构造函数：初始化网络管理器
HttpClient::HttpClient(QObject *parent)
    : QObject(parent)
    , m_manager(new QNetworkAccessManager(this))
{
    connect(m_manager, &QNetworkAccessManager::finished,
            this, &HttpClient::onReplyFinished);
}

void HttpClient::setBaseUrl(const QString &url) {
    m_baseUrl = url;
}

void HttpClient::setToken(const QString &token) {
    m_token = token;
}

// 创建请求：拼接完整 URL、设置 Content-Type、添加 Bearer Token
QNetworkRequest HttpClient::createRequest(const QString &path) const {
    QNetworkRequest request(QUrl(m_baseUrl + path));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    if (!m_token.isEmpty()) {
        request.setRawHeader("Authorization", ("Bearer " + m_token).toUtf8());
    }
    return request;
}

// GET 请求：拼接查询参数后发送
void HttpClient::get(const QString &path, const QUrlQuery &params) {
    QString fullPath = path;
    if (!params.isEmpty()) {
        fullPath += "?" + params.toString();
    }
    Logger::instance().httpRequest("GET", fullPath, "");
    auto req = createRequest(fullPath);
    m_manager->get(req);
}

// POST 请求：将 JSON 对象序列化后发送
void HttpClient::post(const QString &path, const QJsonObject &body) {
    QJsonDocument doc(body);
    Logger::instance().httpRequest("POST", path, QString::fromUtf8(doc.toJson(QJsonDocument::Compact)));
    auto req = createRequest(path);
    m_manager->post(req, doc.toJson());
}

// PUT 请求
void HttpClient::put(const QString &path, const QJsonObject &body) {
    QJsonDocument doc(body);
    Logger::instance().httpRequest("PUT", path, QString::fromUtf8(doc.toJson(QJsonDocument::Compact)));
    auto req = createRequest(path);
    req.setRawHeader("Content-Type", "application/json");
    m_manager->put(req, doc.toJson());
}

// DELETE 请求：支持带 Body 的自定义 DELETE
void HttpClient::del(const QString &path, const QJsonObject &body) {
    QJsonDocument doc(body);
    Logger::instance().httpRequest("DELETE", path,
        body.isEmpty() ? "" : QString::fromUtf8(doc.toJson(QJsonDocument::Compact)));
    auto req = createRequest(path);
    QNetworkReply *reply;
    if (!body.isEmpty()) {
        QByteArray data = doc.toJson();
        reply = m_manager->sendCustomRequest(req, "DELETE", data);
    } else {
        reply = m_manager->deleteResource(req);
    }
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onReplyFinished(reply);
    });
}

// 文件上传：构建 multipart/form-data 请求
void HttpClient::uploadFile(const QString &path, const QString &filePath, const QString &type) {
    Logger::instance().httpRequest("UPLOAD", path, QString("file=%1 type=%2").arg(filePath, type));

    QFile *file = new QFile(filePath);
    if (!file->open(QIODevice::ReadOnly)) {
        Logger::instance().httpError(path, "无法打开文件: " + filePath);
        emit requestError("无法打开文件");
        delete file;
        return;
    }

    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart filePart;
    QMimeDatabase mimeDb;
    QString mimeType = mimeDb.mimeTypeForFile(filePath).name();
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, mimeType);
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                       QString("form-data; name=\"file\"; filename=\"%1\"")
                           .arg(QFileInfo(filePath).fileName()));
    filePart.setBodyDevice(file);
    file->setParent(multiPart);

    QHttpPart typePart;
    typePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                       "form-data; name=\"type\"");
    typePart.setBody(type.toUtf8());

    multiPart->append(filePart);
    multiPart->append(typePart);

    auto req = createRequest(path);
    QNetworkReply *reply = m_manager->post(req, multiPart);
    multiPart->setParent(reply);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onReplyFinished(reply);
    });
}

// 响应处理：解析 JSON、提取状态码并发射信号
// 网络错误和 HTTP 错误都通过 requestFinished 发射，保证调用方总能收到回调
void HttpClient::onReplyFinished(QNetworkReply *reply) {
    reply->deleteLater();

    QString url = reply->url().toString();
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QByteArray data = reply->readAll();

    // 网络层错误（连接失败、超时、DNS 解析失败等）：构造错误 JSON 统一传递
    if (reply->error() != QNetworkReply::NoError && statusCode == 0) {
        Logger::instance().httpError(url, reply->errorString());
        emit requestError(reply->errorString());
        QJsonObject errorResp;
        errorResp["code"] = -1;
        errorResp["message"] = QString("网络错误: %1").arg(reply->errorString());
        errorResp["data"] = QJsonObject();
        emit requestFinished(0, errorResp);
        return;
    }

    // 空响应（常见于 DELETE 等无 Body 的请求）
    if (data.isEmpty()) {
        Logger::instance().httpResponse(url, statusCode, "(empty)");
        QJsonObject emptyResp;
        emptyResp["code"] = (statusCode >= 200 && statusCode < 300) ? 0 : statusCode;
        emptyResp["message"] = (statusCode >= 200 && statusCode < 300) ? "ok" : QString("HTTP %1").arg(statusCode);
        emptyResp["data"] = QJsonObject();
        emit requestFinished(statusCode, emptyResp);
        return;
    }

    QString respStr = QString::fromUtf8(data);
    Logger::instance().httpResponse(url, statusCode, respStr);

    QJsonDocument doc = QJsonDocument::fromJson(data);

    if (doc.isObject()) {
        emit requestFinished(statusCode, doc.object());
    } else {
        Logger::instance().httpError(url, "Invalid JSON response");
        emit requestError("无效的JSON响应");
        QJsonObject errorResp;
        errorResp["code"] = -1;
        errorResp["message"] = "服务器返回了无效的响应格式";
        errorResp["data"] = QJsonObject();
        emit requestFinished(0, errorResp);
    }
}
