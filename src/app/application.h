#ifndef APPLICATION_H
#define APPLICATION_H

#include <QObject>

// 应用初始化类：负责应用的全局初始化工作
class Application : public QObject {
    Q_OBJECT
public:
    explicit Application(QObject *parent = nullptr);
    // 执行初始化逻辑，返回是否成功
    bool init();
};

#endif // APPLICATION_H
