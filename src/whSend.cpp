#include "whSend.h"
#include "Webhook.h"
#include "layoutHelper.h"

WhSend::WhSend(QWidget *parent)
    : QWidget(parent)
{
    // Create buttons
    btnSend = new QPushButton(this);

    // Set button sizes
    btnSend->setFixedSize(164, 61);

    // Set pointer cursor for buttons
    btnSend->setCursor(Qt::PointingHandCursor);

    // Apply stylesheets
    btnSend->setStyleSheet(
        "QPushButton {"
        "    background-image: url(:/images/buttons/btnSend.png);"
        "    background-repeat: no-repeat;"
        "    background-position: center;"
        "    border: none;"
        "    color: white;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-image: url(:/images/buttons/btnSendHover.png);"
        "}"
        );

    // Create inputs
    webhookInput = new WebhookUrlInput;
    usernameInput = new UsernameInput;
    avatarInput = new AvatarUrlInput;
    contentInput = new ContentInput;

    // Create webhook manager
    webhookManager = new WebhookManager(this);

    // Create status label
    statusLabel = new QLabel(this);
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setStyleSheet("font: 14pt Arial;");

    // Input layout with proper spacing
    QVBoxLayout* inputLayout = new QVBoxLayout;
    inputLayout->setSpacing(15);  // Increased spacing between fields
    inputLayout->addWidget(webhookInput);
    inputLayout->addWidget(usernameInput);
    inputLayout->addWidget(avatarInput);
    inputLayout->addWidget(contentInput);
    inputLayout->addStretch();

    // Button layout with proper spacing
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(btnSend);
    buttonLayout->addStretch();

    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);  // Increased main layout spacing

    mainLayout->addLayout(inputLayout, 1);  // Allow input area to expand
    mainLayout->addSpacing(20);             // Extra space before buttons
    mainLayout->addLayout(buttonLayout);
    mainLayout->addSpacing(10);             // Space before status label
    mainLayout->addWidget(statusLabel, 0, Qt::AlignCenter);

    // Connect signals
    connect(btnSend, &QPushButton::pressed,
            this, &WhSend::onSendButtonPressed);
}

void WhSend::onSendButtonPressed()
{
    // Get input values
    QString url = webhookInput->text().trimmed();
    QString username = usernameInput->text().trimmed();
    QString avatarUrl = avatarInput->text().trimmed();
    QString content = contentInput->toPlainText().trimmed();

    // Validate inputs
    if (url.isEmpty()) {
        statusLabel->setText("Please enter a webhook URL");
        statusLabel->setStyleSheet("color: red; font: 14pt Arial;");
        return;
    }

    if (username.isEmpty()) {
        username = "Inferno's DCTool";
    }

    if (content.isEmpty()) {
        statusLabel->setText("Please enter message content");
        statusLabel->setStyleSheet("color: red; font: 14pt Arial;");
        return;
    }

    // Show checking status
    statusLabel->setText("Checking webhook...");
    statusLabel->setStyleSheet("color: orange; font: 14pt Arial;");

    // Set up webhook manager
    webhookManager->webhookUrl = url;
    webhookManager->username = username;
    webhookManager->avatarUrl = avatarUrl;
    webhookManager->content = content;

    // Connect to validation completion signal
    disconnect(webhookManager, &WebhookManager::webhookValidationComplete, this, nullptr);
    connect(webhookManager, &WebhookManager::webhookValidationComplete,
            this, &WhSend::onWebhookValidated);

    // Start webhook validation
    webhookManager->checkWebhook();
}

void WhSend::onWebhookValidated(bool isValid, int statusCode, const QString& errorMessage)
{
    if (!isValid) {
        statusLabel->setText(QString("Webhook invalid (Code: %1)").arg(statusCode));
        statusLabel->setStyleSheet("color: red; font: 14pt Arial;");

        if (!errorMessage.isEmpty()) {
            qDebug() << "Webhook validation error:" << errorMessage;
        }
        return;
    }

    statusLabel->setText("Webhook valid, sending message...");
    statusLabel->setStyleSheet("color: blue; font: 14pt Arial;");

    // Connect to send completion
    disconnect(webhookManager, &WebhookManager::webhookSendComplete, this, nullptr);
    connect(webhookManager, &WebhookManager::webhookSendComplete,
            this, &WhSend::onWebhookSent);

    // Send the webhook
    webhookManager->sendWebhook();
}

void WhSend::onWebhookSent(bool success, const QString& errorMessage)
{
    if (success) {
        statusLabel->setText("Message sent successfully!");
        statusLabel->setStyleSheet("color: green; font: 14pt Arial;");
    } else {
        statusLabel->setText("Failed to send message");
        statusLabel->setStyleSheet("color: red; font: 14pt Arial;");
        if (!errorMessage.isEmpty()) {
            qDebug() << "Webhook send error:" << errorMessage;
        }
    }
}
