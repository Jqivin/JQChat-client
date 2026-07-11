#include "application.h"

// 构造函数
Application::Application(QObject *parent)
    : QObject(parent)
{
}

// 初始化应用（预留）
bool Application::init() {
    return true;
}
