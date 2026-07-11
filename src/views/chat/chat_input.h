#ifndef CHAT_INPUT_H
#define CHAT_INPUT_H

#include <QWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QKeyEvent>

// 聊天输入栏：文本输入 + 表情/图片按钮 + 发送按钮
class ChatInput : public QWidget {
    Q_OBJECT
public:
    explicit ChatInput(QWidget *parent = nullptr);

signals:
    void sendText(const QString &text);
    void sendImage(const QString &path);
    void emojiToggled();

private slots:
    void onSendClicked();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    QTextEdit *m_textEdit;
    QPushButton *m_emojiBtn;
    QPushButton *m_imageBtn;
    QPushButton *m_sendBtn;
};

#endif // CHAT_INPUT_H
