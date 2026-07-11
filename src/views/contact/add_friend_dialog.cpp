#include "add_friend_dialog.h"
#include "viewmodels/contact_viewmodel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsDropShadowEffect>

// 构造函数：构建邮箱搜索、结果显示和添加按钮
AddFriendDialog::AddFriendDialog(ContactViewModel *viewModel, QWidget *parent)
    : QDialog(parent)
    , m_viewModel(viewModel)
{
    setWindowTitle("添加好友");
    setFixedSize(380, 240);
    setStyleSheet("QDialog { background: #fff; }");

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(28, 24, 28, 20);
    layout->setSpacing(14);

    // 标题
    auto *title = new QLabel("🔍 添加好友");
    title->setStyleSheet("font-size: 18px; font-weight: bold; color: #333;");
    layout->addWidget(title);

    // 搜索行：邮箱输入 + 搜索按钮
    auto *searchRow = new QHBoxLayout;
    searchRow->setSpacing(8);
    m_emailEdit = new QLineEdit;
    m_emailEdit->setPlaceholderText("输入对方邮箱地址搜索");
    m_emailEdit->setFixedHeight(40);
    m_emailEdit->setStyleSheet(
        "QLineEdit { border: 1px solid #e0e0e0; border-radius: 6px;"
        "padding: 8px 12px; font-size: 13px; background: #fafafa; }"
        "QLineEdit:focus { border-color: #1197fa; background: #fff; }");
    searchRow->addWidget(m_emailEdit, 1);

    m_searchBtn = new QPushButton("搜索");
    m_searchBtn->setFixedHeight(40);
    m_searchBtn->setStyleSheet(
        "QPushButton { background: #1197fa; color: #fff; border: none;"
        "border-radius: 6px; padding: 8px 18px; font-size: 13px; font-weight: bold; }"
        "QPushButton:hover { background: #06ad56; }"
        "QPushButton:pressed { background: #05944a; }");
    m_searchBtn->setCursor(Qt::PointingHandCursor);
    searchRow->addWidget(m_searchBtn);
    layout->addLayout(searchRow);

    // 结果显示区域
    m_resultLabel = new QLabel;
    m_resultLabel->setAlignment(Qt::AlignCenter);
    m_resultLabel->setWordWrap(true);
    m_resultLabel->setStyleSheet("font-size: 13px; color: #666; padding: 8px;"
                                  "background: #fafafa; border-radius: 6px;"
                                  "min-height: 40px;");
    layout->addWidget(m_resultLabel);

    // 添加按钮（搜索结果后显示）
    m_addBtn = new QPushButton("添 加 好 友");
    m_addBtn->hide();
    m_addBtn->setFixedHeight(40);
    m_addBtn->setStyleSheet(
        "QPushButton { background: #1197fa; color: #fff; border: none;"
        "border-radius: 6px; font-size: 14px; font-weight: bold; }"
        "QPushButton:hover { background: #06ad56; }"
        "QPushButton:pressed { background: #05944a; }");
    m_addBtn->setCursor(Qt::PointingHandCursor);
    layout->addWidget(m_addBtn);

    layout->addStretch();

    connect(m_searchBtn, &QPushButton::clicked, this, &AddFriendDialog::onSearchClicked);
    connect(m_addBtn, &QPushButton::clicked, this, &AddFriendDialog::onAddClicked);
    connect(m_viewModel, &ContactViewModel::searchResult,
            this, &AddFriendDialog::onSearchResult);
    connect(m_viewModel, &ContactViewModel::friendRequestAdded,
            this, &AddFriendDialog::onFriendAdded);
}

// 触发搜索：获取邮箱文本调用 ViewModel 搜索
void AddFriendDialog::onSearchClicked() {
    QString email = m_emailEdit->text().trimmed();
    if (email.isEmpty()) return;
    m_viewModel->searchByEmail(email);
}

// 触发添加好友请求
void AddFriendDialog::onAddClicked() {
    if (m_searchedInfo.user_id > 0) {
        m_viewModel->sendFriendRequest(m_searchedInfo.user_id);
    }
}

// 搜索结果展示：找到则显示用户信息并显示添加按钮
void AddFriendDialog::onSearchResult(const FriendInfo &info, bool found) {
    if (found) {
        m_searchedInfo = info;
        m_resultLabel->setText(QString("👤 用户: %1\n📧 邮箱: %2")
            .arg(info.nickname, info.email));
        m_resultLabel->setStyleSheet("font-size: 13px; color: #333; padding: 8px;"
                                      "background: #e8f5e9; border-radius: 6px;"
                                      "min-height: 40px;");
        m_addBtn->show();
    } else {
        m_resultLabel->setText("未找到该用户");
        m_resultLabel->setStyleSheet("font-size: 13px; color: #e74c3c; padding: 8px;"
                                      "background: #fafafa; border-radius: 6px;"
                                      "min-height: 40px;");
        m_addBtn->hide();
    }
}

// 好友添加结果反馈
void AddFriendDialog::onFriendAdded(bool success, const QString &msg) {
    if (success) {
        m_resultLabel->setText("✅ " + msg);
        m_resultLabel->setStyleSheet("font-size: 13px; color: #1197fa; padding: 8px;"
                                      "background: #e8f5e9; border-radius: 6px;"
                                      "min-height: 40px;");
        m_addBtn->hide();
    } else {
        m_resultLabel->setText("❌ " + msg);
        m_resultLabel->setStyleSheet("font-size: 13px; color: #e74c3c; padding: 8px;"
                                      "background: #fafafa; border-radius: 6px;"
                                      "min-height: 40px;");
    }
}
