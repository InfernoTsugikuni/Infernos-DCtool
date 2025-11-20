#ifndef WHSEND_H
#define WHSEND_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include "layoutHelper.h"

// Forward declarations
class WebhookManager;

class WhSend : public QWidget
{
    Q_OBJECT

public:
    explicit WhSend(QWidget *parent = nullptr);

signals:
    void switchToHome();

private slots:
    void onSendButtonPressed();
    void onWebhookValidated(bool isValid, int statusCode, const QString& errorMessage);
    void onWebhookSent(bool success, const QString& errorMessage);

private:
    // UI Elements
    QPushButton *btnSend;
    QPushButton *btnJson;
    QPushButton *homeButton;
    QLabel *statusLabel;

    // Input widgets
    WebhookUrlInput *webhookInput;
    UsernameInput *usernameInput;
    AvatarUrlInput *avatarInput;
    ContentInput *contentInput;

    // Webhook manager
    WebhookManager *webhookManager;
};

#endif // WHSEND_H
