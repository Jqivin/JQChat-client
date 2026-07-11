#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QList>
#include "models/message_model.h"
#include "network/proto.h"

// 本地 SQLite 数据库管理类：缓存消息、会话和用户信息
class LocalDatabase : public QObject {
    Q_OBJECT
public:
    // 获取单例实例
    static LocalDatabase* instance();
    // 初始化数据库连接，创建表结构
    bool init(const QString &dbPath);

    // 批量保存消息到本地缓存
    void saveMessages(const QList<MessageData> &msgs);
    // 加载两个用户之间的历史消息（分页）
    QList<MessageData> loadMessages(quint64 uid1, quint64 uid2, int limit = 50);

    // 保存或更新单个会话
    void saveConversation(const ConversationData &conv);
    // 加载所有会话列表（按时间倒序）
    QList<ConversationData> loadConversations();

    // 保存当前登录用户信息
    void saveUserInfo(const UserInfo &info);
    // 加载缓存的用户信息
    UserInfo loadUserInfo();

private:
    explicit LocalDatabase(QObject *parent = nullptr);
    static LocalDatabase *s_instance;
    QSqlDatabase m_db;
    void createTables();
};

#endif // DATABASE_H
