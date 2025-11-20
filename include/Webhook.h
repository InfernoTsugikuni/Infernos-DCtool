#pragma once
#ifndef WEBHOOK_H
#define WEBHOOK_H

#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QObject>

class WebhookManager : public QObject {
    Q_OBJECT

public:
    explicit WebhookManager(QObject* parent = nullptr);
    ~WebhookManager();

    // Setters
    void setWebhookUrl(const QString& url);
    void setUsername(const QString& name);
    void setAvatarUrl(const QString& url);
    void setContent(const QString& message);

    // Main functions
    void sendWebhook();
    void checkWebhook();
    void startSpamWebhook();
    void stopSpamWebhook();

    // Webhook parameters
    QString webhookUrl;
    QString username;
    QString avatarUrl;
    QString content;

public slots:
    void setSkipValidation(bool skip);

signals:
    void webhookValidationComplete(bool isValid, int statusCode, const QString& errorMessage);
    void webhookSendComplete(bool success, const QString& errorMessage);
    void spamStatusChanged(const QString& message);

private slots:
    void onSendFinished(QNetworkReply* reply);
    void onCheckFinished(QNetworkReply* reply);
    void onSpamTimer();

    void refreshWebhookValidation();
    QString getCacheStatus() const;

private:
    QNetworkAccessManager* networkManager;
    QTimer* spamTimer;
    bool isSpamming;

    QString createJsonPayload();
    QNetworkRequest createRequest();

    QString validateAndFixUrl(const QString& url);

    struct WebhookCache {
        QString url;
        bool isValid;
        int statusCode;
        QDateTime lastChecked;
        QString errorMessage;
    };

    WebhookCache cache;
    static const int CACHE_VALIDITY_MINUTES = 5;
    bool skipValidation;

    bool isCacheValid() const;
    void updateCache(bool isValid, int statusCode, const QString& error);
    void sendWebhookDirect();
    void validateAndSend();
    void onValidateAndSendFinished(QNetworkReply* reply);
};

#endif // WEBHOOK_H
