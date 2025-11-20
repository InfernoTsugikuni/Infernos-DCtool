#ifndef WHCHECK_H
#define WHCHECK_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDebug>
#include "layoutHelper.h"
#include "Webhook.h"

class WhCheck : public QWidget
{
    Q_OBJECT

public:
    explicit WhCheck(QWidget *parent = nullptr);

signals:
    void switchToHome();  // Signal to request page change to home

private slots:
    void onCheckButtonPressed();
    void onWebhookValidationComplete(bool isValid, int statusCode, const QString& errorMessage);

private:
    QPushButton *homeButton;
    QPushButton *checkButton;
    QLineEdit *webhookURL;

    QLabel *statusLabel;
    QWidget *statusIcon;
    QLabel *questionMarkLabel;
    QLabel *loadingLabel;

    QString whURL;

    URLInput* urlInput;
    WebhookManager* webhookManager;  // Make it a member variable

    void updateStatusDisplay(bool isValid, int statusCode, const QString& errorMessage);
    void setQuestionMarkState();
};

#endif // WHCHECK_H
