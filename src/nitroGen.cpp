// nitroGen.cpp
#include "nitroGen.h"
#include <thread>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFileDialog>
#include <QGroupBox>

NitroGen::NitroGen(QWidget *parent)
    : QWidget(parent), isGenerating(false), totalGenerated(0), validLinksFound(0), threadCount(4) {
    setupUI();
    setupStyling();
    networkManager = new QNetworkAccessManager(this);
    validationTimer = new QTimer(this);
    validationTimer->setInterval(50);
    connect(validationTimer, &QTimer::timeout, this, &NitroGen::onValidationTimer);
    // Welcome
    logToConsole("Discord Nitro Generator Console v3.0", "[SYSTEM]");
    logToConsole("Type 'help' for available commands", "[SYSTEM]");
    logToConsole("Ready.", "[SYSTEM]");
}

NitroGen::~NitroGen() {
    stopContinuousGeneration();
}

void NitroGen::setupUI() {
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(6);

    // Console output - compact for 600x450
    consoleOutput = new QPlainTextEdit(this);
    consoleOutput->setReadOnly(true);
    consoleOutput->setMaximumBlockCount(3000);
    consoleOutput->setMinimumHeight(200);
    consoleOutput->setMaximumHeight(250);
    mainLayout->addWidget(consoleOutput);

    // Generated link display section - compact
    generatedLinkLabel = new QLabel(this);
    generatedLinkLabel->setWordWrap(true);
    generatedLinkLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    generatedLinkLabel->hide();

    copyLinkButton = new QPushButton("Copy", this);
    copyLinkButton->hide();
    copyLinkButton->setCursor(Qt::PointingHandCursor);
    connect(copyLinkButton, &QPushButton::clicked, [this]() { copyToClipboard(lastGeneratedLink); });

    QHBoxLayout *linkLayout = new QHBoxLayout();
    linkLayout->setSpacing(6);
    linkLayout->addWidget(generatedLinkLabel, 1);
    linkLayout->addWidget(copyLinkButton);
    mainLayout->addLayout(linkLayout);

    // Webhook section - streamlined
    QGroupBox *webhookGroup = new QGroupBox("Webhook", this);
    QVBoxLayout *webhookLayout = new QVBoxLayout(webhookGroup);
    webhookLayout->setContentsMargins(6, 6, 6, 6);
    webhookLayout->setSpacing(4);

    QHBoxLayout *hookLayout = new QHBoxLayout();
    webhookInput = new QLineEdit(this);
    webhookInput->setPlaceholderText("Discord webhook URL...");
    sendButton = new QPushButton("Send", this);
    sendButton->setCursor(Qt::PointingHandCursor);
    connect(sendButton, &QPushButton::clicked, this, &NitroGen::sendToWebhook);

    hookLayout->addWidget(webhookInput, 1);
    hookLayout->addWidget(sendButton);
    webhookLayout->addLayout(hookLayout);
    mainLayout->addWidget(webhookGroup);

    // File save section - streamlined
    QGroupBox *saveGroup = new QGroupBox("Save File", this);
    QVBoxLayout *saveGroupLayout = new QVBoxLayout(saveGroup);
    saveGroupLayout->setContentsMargins(6, 6, 6, 6);
    saveGroupLayout->setSpacing(4);

    QHBoxLayout *saveLayout = new QHBoxLayout();
    QLineEdit *saveInput = new QLineEdit(this);
    saveInput->setPlaceholderText("Optional: save path");
    QPushButton *browseButton = new QPushButton("Browse", this);
    browseButton->setCursor(Qt::PointingHandCursor);

    connect(browseButton, &QPushButton::clicked, [this, saveInput]() {
        QString path = QFileDialog::getSaveFileName(this, "Select Save File", QString(), "Text Files (*.txt);;All Files (*)");
        if (!path.isEmpty()) {
            saveInput->setText(path);
            saveFilePath = path;
            logToConsole(QString("Save file set: %1").arg(path), "[SYSTEM]");
        }
    });

    saveLayout->addWidget(saveInput, 1);
    saveLayout->addWidget(browseButton);
    saveGroupLayout->addLayout(saveLayout);
    mainLayout->addWidget(saveGroup);

    // Command input section - compact terminal
    QGroupBox *terminalGroup = new QGroupBox("Terminal", this);
    QVBoxLayout *terminalLayout = new QVBoxLayout(terminalGroup);
    terminalLayout->setContentsMargins(6, 6, 6, 6);
    terminalLayout->setSpacing(4);

    QHBoxLayout *inputLayout = new QHBoxLayout();
    QLabel *promptLabel = new QLabel("nitrogen@console:~$", this);
    promptLabel->setStyleSheet("color: #ff6b00; font-weight: bold; font-size: 10px;");
    commandInput = new QLineEdit(this);
    commandInput->setPlaceholderText("Enter command...");

    inputLayout->addWidget(promptLabel);
    inputLayout->addWidget(commandInput, 1);
    terminalLayout->addLayout(inputLayout);
    mainLayout->addWidget(terminalGroup);

    connect(commandInput, &QLineEdit::returnPressed, this, &NitroGen::handleCommand);
    commandInput->setFocus();
}

void NitroGen::setupStyling() {
    // Main widget styling - optimized for 600x450
    this->setStyleSheet(R"(
        QWidget {
            background: transparent;
            color: #ffffff;
            font-family: 'Segoe UI', Arial, sans-serif;
            font-size: 10px;
        }

        QGroupBox {
            font-weight: bold;
            border: 1px solid #ff6b00;
            border-radius: 8px;
            margin-top: 8px;
            padding-top: 6px;
            background: transparent;
        }

        QGroupBox::title {
            subcontrol-origin: margin;
            left: 8px;
            padding: 0 6px 0 6px;
            color: #ff6b00;
            font-size: 11px;
        }

        QLabel {
            color: #ffffff;
            font-size: 10px;
            background: transparent;
        }
    )");

    // Console output styling - compact for small window
    consoleOutput->setStyleSheet(R"(
        QPlainTextEdit {
            background: rgba(13, 13, 13, 0.9);
            color: #00ff41;
            border: 1px solid #ff6b00;
            border-radius: 8px;
            padding: 6px;
            font-family: 'Consolas', 'Courier New', monospace;
            font-size: 9px;
            line-height: 1.1;
            selection-background-color: rgba(255, 107, 0, 0.3);
        }

        QScrollBar:vertical {
            background: transparent;
            width: 10px;
            border-radius: 5px;
            margin: 0;
        }

        QScrollBar::handle:vertical {
            background: rgba(255, 107, 0, 0.7);
            border-radius: 5px;
            min-height: 20px;
            margin: 1px;
        }

        QScrollBar::handle:vertical:hover {
            background: #ff6b00;
        }

        QScrollBar::handle:vertical:pressed {
            background: #e55a00;
        }

        QScrollBar::add-line:vertical,
        QScrollBar::sub-line:vertical {
            height: 0px;
        }
    )");

    // Command input styling - compact
    commandInput->setStyleSheet(R"(
        QLineEdit {
            background: rgba(26, 26, 26, 0.8);
            color: #ffffff;
            border: 1px solid #ff6b00;
            border-radius: 6px;
            padding: 4px 8px;
            font-family: 'Consolas', 'Courier New', monospace;
            font-size: 10px;
            font-weight: bold;
        }

        QLineEdit:focus {
            border-color: #ff8533;
            background: rgba(13, 13, 13, 0.9);
        }

        QLineEdit::placeholder {
            color: #888888;
            font-style: italic;
        }
    )");

    // Webhook input styling
    webhookInput->setStyleSheet(R"(
        QLineEdit {
            background: rgba(42, 42, 42, 0.8);
            color: #ffffff;
            border: 1px solid #ff6b00;
            border-radius: 6px;
            padding: 4px 8px;
            font-size: 9px;
        }

        QLineEdit:focus {
            border-color: #ff8533;
            background: rgba(26, 26, 26, 0.9);
        }

        QLineEdit::placeholder {
            color: #aaaaaa;
        }
    )");

    // Button styling - compact for small window
    QString buttonStyle = R"(
        QPushButton {
            background: rgba(64, 64, 64, 0.8);
            color: #ffffff;
            border: 1px solid #ff6b00;
            border-radius: 6px;
            padding: 4px 8px;
            font-size: 9px;
            font-weight: bold;
            min-width: 60px;
            min-height: 20px;
        }

        QPushButton:hover {
            background: rgba(255, 107, 0, 0.3);
            border-color: #ff8533;
        }

        QPushButton:pressed {
            background: rgba(255, 107, 0, 0.5);
            border-color: #e55a00;
        }
    )";

    // Apply button styling
    copyLinkButton->setStyleSheet(buttonStyle);
    sendButton->setStyleSheet(buttonStyle);

    // Generated link label styling
    generatedLinkLabel->setStyleSheet(R"(
        QLabel {
            background: rgba(13, 40, 24, 0.9);
            color: #00ff41;
            border: 1px solid #ff6b00;
            border-radius: 6px;
            padding: 4px 8px;
            font-family: 'Consolas', monospace;
            font-size: 9px;
            font-weight: bold;
        }
    )");
}

void NitroGen::handleCommand() {
    QString cmd = commandInput->text().trimmed();
    if (cmd.isEmpty()) return;
    logToConsole(QString("nitrogen@console:~$ %1").arg(cmd), "");
    processCommand(cmd);
    commandInput->clear();
    scrollToBottom();
}

void NitroGen::processCommand(const QString &command) {
    QString cmd = command.toLower().trimmed();
    if (cmd.startsWith("gen")) {
        // Optionally allow "gen N" to set thread count
        QStringList parts = cmd.split(' ');
        if (parts.size() == 2) {
            bool ok; int n = parts[1].toInt(&ok);
            if (ok && n>0 && n<=16) threadCount = n;
        }
        executeGenCommand();
    } else if (cmd == "stop") {
        executeStopCommand();
    } else if (cmd == "clear") {
        executeClearCommand();
    } else if (cmd == "help") {
        executeHelpCommand();
    } else {
        executeUnknownCommand(command);
    }
}

void NitroGen::executeGenCommand() {
    if (isGenerating) {
        logToConsole("Generation already running! Use 'stop' to halt.", "[GEN]");
        return;
    }
    startContinuousGeneration();
}

void NitroGen::executeStopCommand() {
    if (!isGenerating) {
        logToConsole("No generation process running.", "[STOP]");
        return;
    }
    stopContinuousGeneration();
}

void NitroGen::executeClearCommand() {
    consoleOutput->clear();
    logToConsole("Console cleared.", "[SYSTEM]");
    generatedLinkLabel->hide(); copyLinkButton->hide();
}

void NitroGen::executeHelpCommand() {
    unsigned int hardwareThreads = std::thread::hardware_concurrency();

    // Use hardware thread count or fallback to 16 if detection fails
    unsigned int maxThreads = (hardwareThreads == 0) ? 16 : hardwareThreads;

    logToConsole("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━", "[HELP]");
    logToConsole("🔧 AVAILABLE COMMANDS:", "[HELP]");
    logToConsole("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━", "[HELP]");
    logToConsole(QString("⚡ gen [threads] - Start generation (max %1 threads)").arg(maxThreads), "[HELP]");
    logToConsole("🛑 stop         - Stop generation process", "[HELP]");
    logToConsole("🧹 clear        - Clear console output", "[HELP]");
    logToConsole("❓ help         - Show this help message", "[HELP]");
    logToConsole("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━", "[HELP]");
    logToConsole("💡 Valid links are auto-highlighted and sent if webhook is set.", "[HELP]");
    logToConsole("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━", "[HELP]");
}

void NitroGen::executeUnknownCommand(const QString &command) {
    logToConsole(QString("❌ Unknown command: '%1'. Type 'help' for available commands").arg(command), "[ERROR]");
}

void NitroGen::startContinuousGeneration() {
    isGenerating = true;
    totalGenerated = 0;
    validLinksFound = 0;
    logToConsole(QString("🚀 Starting generation with %1 threads...").arg(threadCount), "[GEN]");
    logToConsole("⚙️  Initializing generator threads...", "[GEN]");

    // Launch threads
    for (int i=0;i<threadCount;++i) {
        NitroGeneratorThread *thr = new NitroGeneratorThread(this);
        connect(thr, &NitroGeneratorThread::codeGenerated, this, &NitroGen::onCodeGenerated, Qt::QueuedConnection);
        thr->start();
        generatorThreads.append(thr);
    }
    validationTimer->start();
    logToConsole("✅ Generation started successfully!", "[GEN]");
}

void NitroGen::stopContinuousGeneration() {
    isGenerating = false;
    logToConsole("🛑 Stopping generation threads...", "[STOP]");

    for (auto thr: generatorThreads) {
        thr->stop();
    }
    for (auto thr: generatorThreads) {
        thr->wait(); delete thr;
    }
    generatorThreads.clear();
    validationTimer->stop();

    logToConsole(QString("📊 Generation stopped. Total Generated: %1, Valid Found: %2").arg(totalGenerated).arg(validLinksFound), "[STOP]");
}

void NitroGen::onCodeGenerated(const QString &link) {
    // Enqueue for validation
    pendingMutex.lock();
    pendingValidation.append(link);
    pendingMutex.unlock();
    totalGenerated++;
    if (totalGenerated % 100 == 0) {
        logToConsole(QString("📈 Generated %1 codes, found %2 valid links so far...").arg(totalGenerated).arg(validLinksFound), "[GEN]");
    }
}

void NitroGen::onValidationTimer() {
    pendingMutex.lock();
    if (pendingValidation.isEmpty()) { pendingMutex.unlock(); return; }
    QString link = pendingValidation.takeFirst();
    pendingMutex.unlock();
    validateNitroLink(link);
}

void NitroGen::validateNitroLink(const QString &link) {
    // Extract code
    QString code = link.mid(link.lastIndexOf('/')+1);
    QUrl url(QString("https://discord.com/api/v9/entitlements/gift-codes/%1").arg(code));
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36");
    QNetworkReply *reply = networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, &NitroGen::onNetworkReply);
    // attach link via property
    reply->setProperty("nitroLink", link);
}

void NitroGen::onNetworkReply() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    QString link = reply->property("nitroLink").toString();
    bool valid = false;
    if (reply->error() == QNetworkReply::NoError) {
        int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (status == 200) valid = true;
    }
    reply->deleteLater();
    if (valid) {
        validLinksFound++;
        logToConsole(QString("🎁 VALID NITRO LINK FOUND: %1").arg(link), "[VALID]");
        displayGeneratedLink(link, true);
        if (!webhookInput->text().trimmed().isEmpty()) {
            lastGeneratedLink = link;
            sendToWebhook();
        }
        if (!saveFilePath.isEmpty()) saveValidLink(link);
    }
}

void NitroGen::displayGeneratedLink(const QString &link, bool /*isValid*/) {
    lastGeneratedLink = link;
    QString txt = QString("✅ VALID LINK: %1").arg(link);
    generatedLinkLabel->setText(txt);
    generatedLinkLabel->show(); copyLinkButton->show();
}

void NitroGen::sendToWebhook() {
    QString webhookUrl = webhookInput->text().trimmed();
    if (webhookUrl.isEmpty()) {
        logToConsole("❌ Webhook URL is empty!", "[WEBHOOK]");
        return;
    }
    if (lastGeneratedLink.isEmpty()) {
        logToConsole("❌ No link to send!", "[WEBHOOK]");
        return;
    }

    logToConsole("📤 Sending to webhook...", "[WEBHOOK]");

    QUrl url(webhookUrl);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject json;
    json["content"] = QString("🎁 **Valid Nitro Link Found!** 🎁\n```\n%1\n```").arg(lastGeneratedLink);
    json["username"] = "Nitro Generator";

    QNetworkReply *reply = networkManager->post(request, QJsonDocument(json).toJson());
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            logToConsole("✅ Sent to webhook successfully!", "[WEBHOOK]");
        } else {
            logToConsole(QString("❌ Webhook send failed: %1").arg(reply->errorString()), "[WEBHOOK]");
        }
        reply->deleteLater();
    });
}

void NitroGen::saveValidLink(const QString &link) {
    QFile file(saveFilePath);
    if (file.open(QFile::Append | QFile::Text)) {
        QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        file.write(QString("[%1] %2\n").arg(timestamp, link).toUtf8());
        file.close();
        logToConsole("💾 Valid link saved to file!", "[SAVE]");
    } else {
        logToConsole("❌ Failed to save link to file!", "[SAVE]");
    }
}

void NitroGen::copyToClipboard(const QString &text) {
    QApplication::clipboard()->setText(text);
    logToConsole("📋 Link copied to clipboard!", "[SYSTEM]");
}

void NitroGen::logToConsole(const QString &message, const QString &prefix) {
    QString ts = getCurrentTimestamp();
    QString coloredPrefix = prefix;

    // Add color coding for different message types
    if (prefix == "[VALID]") coloredPrefix = "🎁[VALID]";
    else if (prefix == "[ERROR]") coloredPrefix = "❌[ERROR]";
    else if (prefix == "[GEN]") coloredPrefix = "⚡[GEN]";
    else if (prefix == "[STOP]") coloredPrefix = "🛑[STOP]";
    else if (prefix == "[SYSTEM]") coloredPrefix = "🔧[SYSTEM]";
    else if (prefix == "[WEBHOOK]") coloredPrefix = "📤[WEBHOOK]";
    else if (prefix == "[SAVE]") coloredPrefix = "💾[SAVE]";
    else if (prefix == "[HELP]") coloredPrefix = "❓[HELP]";

    QString entry = coloredPrefix.isEmpty() ?
        QString("[%1] %2").arg(ts, message) :
        QString("[%1] %2 %3").arg(ts, coloredPrefix, message);

    consoleOutput->appendPlainText(entry);
    scrollToBottom();
}

void NitroGen::scrollToBottom() {
    QTextCursor c = consoleOutput->textCursor();
    c.movePosition(QTextCursor::End);
    consoleOutput->setTextCursor(c);
    consoleOutput->ensureCursorVisible();
}

QString NitroGen::getCurrentTimestamp() {
    return QDateTime::currentDateTime().toString("hh:mm:ss");
}

// Add the missing slot implementations if they're referenced elsewhere
void NitroGen::onGenerationTimer() {
    // Implementation if needed
}

void NitroGen::onWebhookSendComplete(bool success, const QString &error) {
    // Implementation if needed
    if (success) {
        logToConsole("✅ Webhook sent successfully!", "[WEBHOOK]");
    } else {
        logToConsole(QString("❌ Webhook failed: %1").arg(error), "[WEBHOOK]");
    }
}
