#ifndef WHSPAM_H
#define WHSPAM_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include "layoutHelper.h"

// Forward declarations
class WebhookManager;

class WhSpam : public QWidget
{
    Q_OBJECT

public:
    explicit WhSpam(QWidget *parent = nullptr);

signals:
    void switchToHome();

private slots:
    void onStartButtonPressed();
    void onStopButtonPressed();
    void onWebhookValidated(bool isValid, int statusCode, const QString& errorMessage);
    void onSpamStatusChanged(const QString& message);

private:
    // UI Elements
    QPushButton *btnStart;
    QPushButton *btnStop;
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

    // State
    bool isSpamming;
};

#endif // WHSPAM_H
