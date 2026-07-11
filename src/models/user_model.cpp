#include "user_model.h"

UserModel::UserModel(QObject *parent)
    : QObject(parent)
{
}

// 设置用户信息并通知 UI 更新
void UserModel::setUserInfo(const UserInfo &info) {
    m_userInfo = info;
    emit userChanged();
}
