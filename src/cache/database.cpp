#include "database.h"
#include "network/api_manager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

LocalDatabase* LocalDatabase::s_instance = nullptr;

// 获取单例实例（懒加载）
LocalDatabase* LocalDatabase::instance() {
    if (!s_instance) {
        s_instance = new LocalDatabase();
    }
    return s_instance;
}

LocalDatabase::LocalDatabase(QObject *parent)
    : QObject(parent)
{
}

// 初始化：打开 SQLite 数据库并创建表
bool LocalDatabase::init(const QString &dbPath) {
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbPath);

    if (!m_db.open()) {
        qWarning() << "打开本地数据库失败:" << m_db.lastError().text();
        return false;
    }

    createTables();
    return true;
}

// 创建消息表、会话表和用户信息表
void LocalDatabase::createTables() {
    QSqlQuery query(m_db);

    query.exec(R"(CREATE TABLE IF NOT EXISTS messages (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        msg_id TEXT UNIQUE,
        from_uid INTEGER,
        to_uid INTEGER,
        msg_type INTEGER,
        content TEXT,
        file_url TEXT,
        created_at TEXT,
        status INTEGER DEFAULT 1
    ))");

    query.exec(R"(CREATE TABLE IF NOT EXISTS conversations (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        target_uid INTEGER UNIQUE,
        last_msg TEXT,
        last_time TEXT,
        unread INTEGER DEFAULT 0
    ))");

    query.exec(R"(CREATE TABLE IF NOT EXISTS user_info (
        key TEXT PRIMARY KEY,
        value TEXT
    ))");
}

// 批量插入消息到本地数据库（重复 msg_id 则忽略）
void LocalDatabase::saveMessages(const QList<MessageData> &msgs) {
    QSqlQuery query(m_db);
    query.prepare(R"(INSERT OR IGNORE INTO messages
        (msg_id, from_uid, to_uid, msg_type, content, file_url, created_at, status)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?))");

    for (const auto &msg : msgs) {
        query.addBindValue(msg.msg_id);
        query.addBindValue(static_cast<qint64>(msg.from_uid));
        query.addBindValue(static_cast<qint64>(msg.to_uid));
        query.addBindValue(msg.msg_type);
        query.addBindValue(msg.content);
        query.addBindValue(msg.file_url);
        query.addBindValue(msg.created_at.toString(Qt::ISODate));
        query.addBindValue(msg.status);
        query.exec();
    }
}

// 加载两个用户之间的最近消息（按时间倒序并反转）
QList<MessageData> LocalDatabase::loadMessages(quint64 uid1, quint64 uid2, int limit) {
    QList<MessageData> msgs;
    QSqlQuery query(m_db);
    query.prepare(R"(SELECT * FROM messages
        WHERE (from_uid = ? AND to_uid = ?) OR (from_uid = ? AND to_uid = ?)
        ORDER BY id DESC LIMIT ?)");
    query.addBindValue(static_cast<qint64>(uid1));
    query.addBindValue(static_cast<qint64>(uid2));
    query.addBindValue(static_cast<qint64>(uid2));
    query.addBindValue(static_cast<qint64>(uid1));
    query.addBindValue(limit);
    query.exec();

    while (query.next()) {
        MessageData msg;
        msg.msg_id = query.value("msg_id").toString();
        msg.from_uid = query.value("from_uid").toULongLong();
        msg.to_uid = query.value("to_uid").toULongLong();
        msg.msg_type = query.value("msg_type").toInt();
        msg.content = query.value("content").toString();
        msg.file_url = query.value("file_url").toString();
        msg.created_at = QDateTime::fromString(query.value("created_at").toString(), Qt::ISODate);
        msg.status = query.value("status").toInt();
        msgs.prepend(msg);
    }
    return msgs;
}

// 保存或替换会话记录（INSERT OR REPLACE）
void LocalDatabase::saveConversation(const ConversationData &conv) {
    QSqlQuery query(m_db);
    query.prepare(R"(INSERT OR REPLACE INTO conversations
        (target_uid, last_msg, last_time, unread)
        VALUES (?, ?, ?, ?))");
    query.addBindValue(static_cast<qint64>(conv.target_uid));
    query.addBindValue(conv.last_msg);
    query.addBindValue(conv.last_time.toString(Qt::ISODate));
    query.addBindValue(conv.unread);
    query.exec();
}

// 加载所有会话，按最后消息时间倒序
QList<ConversationData> LocalDatabase::loadConversations() {
    QList<ConversationData> convs;
    QSqlQuery query(m_db);
    query.exec("SELECT * FROM conversations ORDER BY last_time DESC");
    while (query.next()) {
        ConversationData conv;
        conv.target_uid = query.value("target_uid").toULongLong();
        conv.last_msg = query.value("last_msg").toString();
        conv.last_time = QDateTime::fromString(query.value("last_time").toString(), Qt::ISODate);
        conv.unread = query.value("unread").toInt();
        convs.append(conv);
    }
    return convs;
}

// 将用户信息以键值对形式存入数据库
void LocalDatabase::saveUserInfo(const UserInfo &info) {
    QSqlQuery query(m_db);
    query.prepare("INSERT OR REPLACE INTO user_info (key, value) VALUES (?, ?)");
    query.addBindValue("user_id");
    query.addBindValue(QString::number(info.user_id));
    query.exec();

    query.prepare("INSERT OR REPLACE INTO user_info (key, value) VALUES (?, ?)");
    query.addBindValue("email");
    query.addBindValue(info.email);
    query.exec();

    query.prepare("INSERT OR REPLACE INTO user_info (key, value) VALUES (?, ?)");
    query.addBindValue("nickname");
    query.addBindValue(info.nickname);
    query.exec();

    query.prepare("INSERT OR REPLACE INTO user_info (key, value) VALUES (?, ?)");
    query.addBindValue("avatar_url");
    query.addBindValue(info.avatar_url);
    query.exec();

    query.prepare("INSERT OR REPLACE INTO user_info (key, value) VALUES (?, ?)");
    query.addBindValue("token");
    query.addBindValue(ApiManager::instance()->token());
    query.exec();
}

// 从数据库中加载缓存的用户信息
UserInfo LocalDatabase::loadUserInfo() {
    UserInfo info;
    QSqlQuery query(m_db);
    query.exec("SELECT key, value FROM user_info");
    while (query.next()) {
        QString key = query.value("key").toString();
        QString val = query.value("value").toString();
        if (key == "user_id") info.user_id = val.toULongLong();
        else if (key == "email") info.email = val;
        else if (key == "nickname") info.nickname = val;
        else if (key == "avatar_url") info.avatar_url = val;
    }
    return info;
}
