#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QMutex>
#include <QDateTime>

// 全局日志工具：单例模式，线程安全
// 日志文件命名：JQChat_2026-07-11_14-30-00.log
class Logger : public QObject {
    Q_OBJECT
public:
    static Logger& instance();

    // 初始化日志文件（应用启动时调用一次）
    void init(const QString &logDir);

    void info(const QString &msg);
    void warn(const QString &msg);
    void error(const QString &msg);

    // HTTP 请求/响应专用日志
    void httpRequest(const QString &method, const QString &url, const QString &body);
    void httpResponse(const QString &url, int statusCode, const QString &body);
    void httpError(const QString &url, const QString &error);

    // WebSocket 专用日志
    void wsEvent(const QString &event, const QString &detail = "");
    void wsSend(const QString &data);
    void wsRecv(const QString &data);

private:
    explicit Logger(QObject *parent = nullptr);
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void writeLine(const QString &level, const QString &msg);

    QFile m_file;
    QTextStream m_stream;
    QMutex m_mutex;
    bool m_initialized = false;
};

#endif // LOGGER_H
