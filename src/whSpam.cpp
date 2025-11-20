#include "whSpam.h"
#include "Webhook.h"
#include "layoutHelper.h"

WhSpam::WhSpam(QWidget *parent)
    : QWidget(parent), isSpamming(false)
{
    // Create buttons
    btnStart = new QPushButton(this);
    btnStop = new QPushButton(this);

    // Set button sizes
    btnStart->setFixedSize(164, 61);
    btnStop->setFixedSize(164, 61);

    // Set pointer cursor for buttons
    btnStart->setCursor(Qt::PointingHandCursor);
    btnStop->setCursor(Qt::PointingHandCursor);

    // Apply stylesheets
    btnStart->setStyleSheet(
        "QPushButton {"
        "    background-image: url(:/images/buttons/SpammerButStart.png);"
        "    background-repeat: no-repeat;"
        "    background-position: center;"
        "    border: none;"
        "    color: white;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-image: url(:/images/buttons/SpammerButStartHover.png);"
        "}"
        );

    btnStop->setStyleSheet(
        "QPushButton {"
        "    background-image: url(:/images/buttons/SpammerButStop.png);"
        "    background-repeat: no-repeat;"
        "    background-position: center;"
        "    border: none;"
        "    color: white;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-image: url(:/images/buttons/SpammerButStopHover.png);"
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
    buttonLayout->addWidget(btnStart);
    buttonLayout->addSpacing(20);
    buttonLayout->addWidget(btnStop);
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
    connect(btnStart, &QPushButton::pressed,
            this, &WhSpam::onStartButtonPressed);

    connect(btnStop, &QPushButton::pressed,
            this, &WhSpam::onStopButtonPressed);

    // Connect webhook manager signals
    connect(webhookManager, &WebhookManager::spamStatusChanged,
            this, &WhSpam::onSpamStatusChanged);

    connect(webhookManager, &WebhookManager::webhookValidationComplete,
            this, &WhSpam::onWebhookValidated);

    // Initial status
    statusLabel->setText("Ready to spam");
    statusLabel->setStyleSheet("color: gray; font: 14pt Arial;");
}

void WhSpam::onStartButtonPressed()
{
    if (isSpamming) {
        statusLabel->setText("Spamming already in progress");
        statusLabel->setStyleSheet("color: orange; font: 14pt Arial;");
        return;
    }

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

    // Start webhook validation
    webhookManager->checkWebhook();
}

void WhSpam::onStopButtonPressed()
{
    if (!isSpamming) {
        statusLabel->setText("No spam in progress");
        statusLabel->setStyleSheet("color: gray; font: 14pt Arial;");
        return;
    }

    // Stop the spam
    webhookManager->stopSpamWebhook();
    isSpamming = false;

    statusLabel->setText("Stopping spam...");
    statusLabel->setStyleSheet("color: orange; font: 14pt Arial;");
}

void WhSpam::onWebhookValidated(bool isValid, int statusCode, const QString& errorMessage)
{
    if (!isValid) {
        statusLabel->setText(QString("Webhook invalid (Code: %1)").arg(statusCode));
        statusLabel->setStyleSheet("color: red; font: 14pt Arial;");

        if (!errorMessage.isEmpty()) {
            qDebug() << "Webhook validation error:" << errorMessage;
        }
        return;
    }

    // Webhook is valid, start spamming
    statusLabel->setText("Webhook valid, starting spam...");
    statusLabel->setStyleSheet("color: blue; font: 14pt Arial;");

    // Start the spam
    webhookManager->startSpamWebhook();
    isSpamming = true;
}

void WhSpam::onSpamStatusChanged(const QString& message)
{
    if (message.contains("started")) {
        statusLabel->setText("Spamming in progress...");
        statusLabel->setStyleSheet("color: green; font: 14pt Arial;");
        isSpamming = true;
    } else if (message.contains("stopped")) {
        statusLabel->setText("Spam stopped");
        statusLabel->setStyleSheet("color: red; font: 14pt Arial;");
        isSpamming = false;
    } else {
        statusLabel->setText(message);
        statusLabel->setStyleSheet("color: gray; font: 14pt Arial;");
    }
}
