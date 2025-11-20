#include "whCheck.h"
#include "Webhook.h"

WhCheck::WhCheck(QWidget *parent)
    : QWidget(parent), questionMarkLabel(nullptr), loadingLabel(nullptr)
{
    // Initialize webhook manager
    webhookManager = new WebhookManager(this);

    // Status display section - now using QWidget for icon
    statusIcon = new QWidget(this);
    statusIcon->setFixedSize(40, 35);
    statusIcon->setStyleSheet(R"(
        QWidget {
            background-image: url();
            background-repeat: no-repeat;
            background-position: center;
            border: 1px solid gray;
            border-radius: 17px;
        }
    )");

    // Status label setup - FIXED ALIGNMENT
    statusLabel = new QLabel("Enter a webhook URL to check", this);
    statusLabel->setStyleSheet("color: gray; font: 16pt Arial;");
    statusLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);  // Changed to AlignTop
    statusLabel->setWordWrap(true);
    statusLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred); // Added expansion

    // URL Input Section
    urlInput = new URLInput(this);
    urlInput->setFixedSize(474, 41);
    urlInput->setStyleSheet(R"(
        QLineEdit {
            border: none;
            background: #101010;
            margin: 0px;
            padding: 0px 12px;
            color: white;
        }
        QLineEdit::placeholder {
            color: white;
        }
    )");
    urlInput->setFrame(false);
    urlInput->setContentsMargins(0, 0, 0, 0);

    // Check Button
    QPushButton* checkButton = new QPushButton(this);
    checkButton->setFixedSize(109, 41);
    checkButton->setCursor(Qt::PointingHandCursor);
    checkButton->setStyleSheet(R"(
        QPushButton {
            background-image: url(:/images/buttons/CheckBut.png);
            border: none;
            margin: 0px;
            padding: 0px;
        }
        QPushButton:hover {
            background-image: url(:/images/buttons/CheckButHover.png);
        }
    )");
    checkButton->setContentsMargins(0, 0, 0, 0);

    // Input Layout
    QHBoxLayout* urlLayout = new QHBoxLayout();
    urlLayout->setSpacing(0);
    urlLayout->setContentsMargins(0, 0, 0, 0);
    urlLayout->addWidget(urlInput);
    urlLayout->addWidget(checkButton);

    // Status Layout - FIXED LAYOUT ALIGNMENT
    QHBoxLayout* statusLayout = new QHBoxLayout();
    statusLayout->setAlignment(Qt::AlignTop);  // Changed to AlignTop
    statusLayout->setSpacing(15);
    statusLayout->setContentsMargins(0, 0, 0, 0);
    statusLayout->addWidget(statusIcon, 0, Qt::AlignTop);  // Explicit top alignment
    statusLayout->addWidget(statusLabel, 1);  // Add stretch factor

    // Main Layout Setup
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    mainLayout->addStretch();
    mainLayout->addLayout(urlLayout);
    mainLayout->addStretch();
    mainLayout->addLayout(statusLayout);
    mainLayout->addStretch();
    mainLayout->setSpacing(30);

    // Signal Connections
    connect(checkButton, &QPushButton::pressed,
            this, &WhCheck::onCheckButtonPressed);
    connect(webhookManager, &WebhookManager::webhookValidationComplete,
            this, &WhCheck::onWebhookValidationComplete);

    // Set initial question mark state
    setQuestionMarkState();
}

void WhCheck::setQuestionMarkState()
{
    statusIcon->setStyleSheet(R"(
        QWidget {
            background-image: url();
            background-repeat: no-repeat;
            background-position: center;
            border: 2px solid gray;
            border-radius: 17px;
        }
    )");

    // Clean up any existing labels first - ADD NULL CHECKS
    if (loadingLabel) {
        loadingLabel->hide();
    }

    // Create or reuse question mark label
    if (!questionMarkLabel) {
        questionMarkLabel = new QLabel("?", statusIcon);
        questionMarkLabel->setStyleSheet("color: gray; font: bold 18pt Arial; border: none;");
        questionMarkLabel->setAlignment(Qt::AlignCenter);
        questionMarkLabel->setGeometry(0, 0, 40, 35);
    }
    questionMarkLabel->show();
}

void WhCheck::onCheckButtonPressed()
{
    QString url = urlInput->text().trimmed();

    if (url.isEmpty()) {
        updateStatusDisplay(false, 0, "Please enter a webhook URL");
        return;
    }

    // Show checking status
    if (questionMarkLabel) {
        questionMarkLabel->hide();
    }
    statusIcon->setStyleSheet(R"(
        QWidget {
            background-image: url();
            background-repeat: no-repeat;
            background-position: center;
            border: 2px solid orange;
            border-radius: 17px;
        }
    )");

    // Create loading indicator
    if (!loadingLabel) {
        loadingLabel = new QLabel("⏳", statusIcon);
        loadingLabel->setStyleSheet("color: orange; font: bold 18pt Arial; border: none;");
        loadingLabel->setAlignment(Qt::AlignCenter);
        loadingLabel->setGeometry(0, 0, 40, 35);
    }
    loadingLabel->show();

    statusLabel->setText("Checking webhook...");
    statusLabel->setStyleSheet("color: orange; font: 16pt Arial;");

    // Set the webhook URL and check it
    webhookManager->webhookUrl = url;
    webhookManager->checkWebhook();
}

void WhCheck::onWebhookValidationComplete(bool isValid, int statusCode, const QString& errorMessage)
{
    updateStatusDisplay(isValid, statusCode, errorMessage);
}

void WhCheck::updateStatusDisplay(bool isValid, int statusCode, const QString& errorMessage)
{
    // Hide any temporary labels
    if (questionMarkLabel) {
        questionMarkLabel->hide();
    }
    if (loadingLabel) {
        loadingLabel->hide();
    }

    if (isValid) {
        statusIcon->setStyleSheet(R"(
            QWidget {
                background-image: url(:/images/icons/ValidWh.png);
                background-repeat: no-repeat;
                background-position: center;
                border: none;
                border-radius: 17px;
            }
        )");
        statusLabel->setText("Webhook is valid and active!");
        statusLabel->setStyleSheet("color: #00FF00; font: 16pt Arial;");
    } else {
        statusIcon->setStyleSheet(R"(
            QWidget {
                background-image: url(:/images/icons/InvalidWh.png);
                background-repeat: no-repeat;
                background-position: center;
                border: none;
                border-radius: 17px;
            }
        )");

        QString message = "Webhook is invalid";
        if (statusCode != 0) {
            message += QString(" (Status: %1)").arg(statusCode);
        }
        if (!errorMessage.isEmpty()) {
            message += QString("\nError: %1").arg(errorMessage);
        }

        statusLabel->setText(message);
        statusLabel->setStyleSheet("color: #FF4444; font: 16pt Arial;");
    }
}
