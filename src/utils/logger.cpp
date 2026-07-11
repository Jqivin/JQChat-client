#include "logger.h"
#include <QDir>
#include <QDebug>

Logger& Logger::instance() {
    static Logger inst;
    return inst;
}

Logger::Logger(QObject *parent)
    : QObject(parent)
{
}

Logger::~Logger() {
    if (m_file.isOpen()) {
        m_stream.flush();
        m_file.close();
    }
}

void Logger::init(const QString &logDir) {
    QMutexLocker locker(&m_mutex);
    if (m_initialized) return;

    QDir().mkpath(logDir);
    QString fileName = QString("JQChat_%1.log")
        .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss"));
    QString filePath = logDir + "/" + fileName;

    m_file.setFileName(filePath);
    if (m_file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        m_stream.setDevice(&m_file);
        m_initialized = true;
        writeLine("INFO", QString("=== JQChat 启动，日志文件: %1 ===").arg(fileName));
        qDebug() << "[Logger] started:" << filePath;
    } else {
        qWarning() << "[Logger] failed to open:" << filePath;
    }
}

void Logger::info(const QString &msg) {
    writeLine("INFO", msg);
}

void Logger::warn(const QString &msg) {
    writeLine("WARN", msg);
}

void Logger::error(const QString &msg) {
    writeLine("ERROR", msg);
}

void Logger::httpRequest(const QString &method, const QString &url, const QString &body) {
    if (body.isEmpty()) {
        writeLine("HTTP", QString("[REQ] %1 %2").arg(method, url));
    } else {
        writeLine("HTTP", QString("[REQ] %1 %2  Body: %3").arg(method, url, body));
    }
}

void Logger::httpResponse(const QString &url, int statusCode, const QString &body) {
    // 截断过长的响应体
    QString shortBody = body;
    if (shortBody.length() > 500) {
        shortBody = shortBody.left(500) + "...";
    }
    writeLine("HTTP", QString("[RESP] %1  HTTP %2  %3").arg(url).arg(statusCode).arg(shortBody));
}

void Logger::httpError(const QString &url, const QString &error) {
    writeLine("HTTP", QString("[ERR] %1  %2").arg(url, error));
}

void Logger::wsEvent(const QString &event, const QString &detail) {
    if (detail.isEmpty()) {
        writeLine("WS", QString("[%1]").arg(event));
    } else {
        writeLine("WS", QString("[%1] %2").arg(event, detail));
    }
}

void Logger::wsSend(const QString &data) {
    QString shortData = data.length() > 500 ? data.left(500) + "..." : data;
    writeLine("WS", QString("[SEND] %1").arg(shortData));
}

void Logger::wsRecv(const QString &data) {
    QString shortData = data.length() > 500 ? data.left(500) + "..." : data;
    writeLine("WS", QString("[RECV] %1").arg(shortData));
}

void Logger::writeLine(const QString &level, const QString &msg) {
    // 注意：此方法调用前需要外部加锁
    if (!m_initialized) return;

    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
    QString line = QString("[%1] [%2] %3\n").arg(timestamp, level, msg);

    m_stream << line;
    m_stream.flush();
}
