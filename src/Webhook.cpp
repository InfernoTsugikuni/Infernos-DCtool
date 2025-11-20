#include "Webhook.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QTimer>

WebhookManager::WebhookManager(QObject* parent)
    : QObject(parent), isSpamming(false) {
    networkManager = new QNetworkAccessManager(this);

    spamTimer = new QTimer(this);
    spamTimer->setSingleShot(false);
    spamTimer->setInterval(1);

    connect(spamTimer, &QTimer::timeout, this, &WebhookManager::onSpamTimer);
}

QString WebhookManager::validateAndFixUrl(const QString& url) {
    QString fixedUrl = url.trimmed();

    // Check if URL is empty
    if (fixedUrl.isEmpty()) {
        return "";
    }

    // Add https:// if no protocol is specified
    if (!fixedUrl.startsWith("http://") && !fixedUrl.startsWith("https://")) {
        fixedUrl = "https://" + fixedUrl;
    }

    // Validate the URL
    QUrl qurl(fixedUrl);
    if (!qurl.isValid()) {
        return "";
    }

    return fixedUrl;
}

WebhookManager::~WebhookManager() {
    stopSpamWebhook();
}

bool WebhookManager::isCacheValid() const {
    if (cache.url != webhookUrl) {
        return false; // URL changed, cache invalid
    }

    if (cache.lastChecked.isNull()) {
        return false; // Never checked
    }

    // Check if cache is still fresh
    QDateTime now = QDateTime::currentDateTime();
    int minutesElapsed = cache.lastChecked.secsTo(now) / 60;

    return minutesElapsed < CACHE_VALIDITY_MINUTES;
}

void WebhookManager::updateCache(bool isValid, int statusCode, const QString& error) {
    cache.url = webhookUrl;
    cache.isValid = isValid;
    cache.statusCode = statusCode;
    cache.errorMessage = error;
    cache.lastChecked = QDateTime::currentDateTime();
}


void WebhookManager::setWebhookUrl(const QString& url) {
    QString validatedUrl = validateAndFixUrl(url);
    if (validatedUrl.isEmpty() && !url.isEmpty()) {
        emit webhookValidationComplete(false, -1, "Invalid URL format");
        return;
    }
    webhookUrl = validatedUrl;
}

void WebhookManager::setUsername(const QString& name) {
    username = name;
}

void WebhookManager::setAvatarUrl(const QString& url) {
    avatarUrl = url;
}

void WebhookManager::setContent(const QString& message) {
    content = message;
}

QString WebhookManager::createJsonPayload() {
    QJsonObject payload;
    if (!username.isEmpty()) payload["username"] = username;
    if (!avatarUrl.isEmpty()) payload["avatar_url"] = avatarUrl;
    if (!content.isEmpty()) payload["content"] = content;

    QJsonDocument doc(payload);
    return doc.toJson(QJsonDocument::Indented);
}

QNetworkRequest WebhookManager::createRequest() {
    QNetworkRequest request;
    request.setUrl(QUrl(webhookUrl));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("User-Agent", "Qt Webhook Client");
    return request;
}

void WebhookManager::sendWebhook() {
    if (webhookUrl.isEmpty()) {
        emit webhookSendComplete(false, "Webhook URL is empty");
        return;
    }

    // Skip validation entirely if configured to do so
    if (skipValidation) {
        sendWebhookDirect();
        return;
    }

    // Use cached validation result if available and fresh
    if (isCacheValid()) {
        if (cache.isValid) {
            sendWebhookDirect();
        } else {
            emit webhookSendComplete(false, "Webhook invalid (cached): " + cache.errorMessage);
        }
        return;
    }

    // Need to validate first, then send
    validateAndSend();
}

void WebhookManager::sendWebhookDirect() {
    QNetworkRequest request = createRequest();
    QByteArray data = createJsonPayload().toUtf8();
    QNetworkReply* reply = networkManager->post(request, data);

    connect(reply, &QNetworkReply::finished, [this, reply]() {
        onSendFinished(reply);
    });
}

void WebhookManager::validateAndSend() {
    if (webhookUrl.isEmpty()) {
        emit webhookSendComplete(false, "Webhook URL is empty");
        return;
    }

    QUrl qurl(webhookUrl);
    if (!qurl.isValid() || qurl.scheme().isEmpty()) {
        QString error = "Invalid URL format";
        updateCache(false, -1, error);
        emit webhookSendComplete(false, error);
        return;
    }

    QNetworkRequest request;
    request.setUrl(qurl);
    request.setRawHeader("User-Agent", "Qt Webhook Client");

    QNetworkReply* reply = networkManager->get(request);

    connect(reply, &QNetworkReply::finished, [this, reply]() {
        onValidateAndSendFinished(reply);
    });
}

void WebhookManager::onValidateAndSendFinished(QNetworkReply* reply) {
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    bool isValid = (reply->error() == QNetworkReply::NoError && statusCode == 200);
    QString error = isValid ? "" : reply->errorString();

    // Update cache with validation result
    updateCache(isValid, statusCode, error);

    reply->deleteLater();

    // Now send the actual webhook if validation passed
    if (isValid) {
        sendWebhookDirect();
    } else {
        emit webhookSendComplete(false, "Webhook validation failed: " + error);
    }
}

void WebhookManager::onSendFinished(QNetworkReply* reply) {
    bool success = (reply->error() == QNetworkReply::NoError);
    QString error = success ? "" : reply->errorString();
    emit webhookSendComplete(success, error);
    reply->deleteLater();
}

void WebhookManager::startSpamWebhook() {
    if (webhookUrl.isEmpty()) {
        emit spamStatusChanged("Webhook URL is empty");
        return;
    }

    if (isSpamming) return;

    isSpamming = true;
    spamTimer->start();
    emit spamStatusChanged("Spam started");
}

void WebhookManager::stopSpamWebhook() {
    if (!isSpamming) return;

    spamTimer->stop();
    isSpamming = false;
    emit spamStatusChanged("Spam stopped");
}

void WebhookManager::onSpamTimer() {
    QNetworkRequest request = createRequest();
    QByteArray data = createJsonPayload().toUtf8();
    QNetworkReply* reply = networkManager->post(request, data);

    connect(reply, &QNetworkReply::finished, [reply]() {
        reply->deleteLater();
    });
}

void WebhookManager::checkWebhook() {
    if (webhookUrl.isEmpty()) {
        emit webhookValidationComplete(false, -1, "URL empty");
        return;
    }

    // Check cache first
    if (isCacheValid()) {
        emit webhookValidationComplete(cache.isValid, cache.statusCode, cache.errorMessage);
        return;
    }

    QUrl qurl(webhookUrl);
    if (!qurl.isValid() || qurl.scheme().isEmpty()) {
        QString error = "Invalid URL format";
        updateCache(false, -1, error);
        emit webhookValidationComplete(false, -1, error);
        return;
    }

    QNetworkRequest request;
    request.setUrl(qurl);
    request.setRawHeader("User-Agent", "Qt Webhook Client");

    QNetworkReply* reply = networkManager->get(request);

    connect(reply, &QNetworkReply::finished, [this, reply]() {
        onCheckFinished(reply);
    });
}


void WebhookManager::onCheckFinished(QNetworkReply* reply) {
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    bool isValid = (reply->error() == QNetworkReply::NoError && statusCode == 200);
    QString error = isValid ? "" : reply->errorString();

    // Update cache
    updateCache(isValid, statusCode, error);

    emit webhookValidationComplete(isValid, statusCode, error);
    reply->deleteLater();
}

void WebhookManager::setSkipValidation(bool skip) {
    skipValidation = skip;
}

// Force cache refresh
void WebhookManager::refreshWebhookValidation() {
    cache = WebhookCache(); // Clear cache
    checkWebhook();
}

QString WebhookManager::getCacheStatus() const {
    if (cache.url != webhookUrl) {
        return "Cache invalid (URL changed)";
    }

    if (cache.lastChecked.isNull()) {
        return "Not validated";
    }

    QDateTime now = QDateTime::currentDateTime();
    int minutesElapsed = cache.lastChecked.secsTo(now) / 60;

    if (minutesElapsed >= CACHE_VALIDITY_MINUTES) {
        return QString("Cache expired (%1 min ago)").arg(minutesElapsed);
    }

    return QString("Cache valid (%1 status, %2 min ago)")
        .arg(cache.isValid ? "OK" : "FAIL")
        .arg(minutesElapsed);
}
