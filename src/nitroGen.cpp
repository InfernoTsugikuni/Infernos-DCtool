#include "nitroGen.h"
#include <thread>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFileDialog>
#include <QGroupBox>
#include <QMutexLocker>
#include <QDir> // Added QDir for path handling

// ==========================================
// NitroGeneratorThread Implementation
// ==========================================

NitroGeneratorThread::NitroGeneratorThread(QObject *parent)
    : QThread(parent), stopFlag(false), pauseFlag(false) {}

void NitroGeneratorThread::run() {
    static const QString chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    const int charsLen = chars.length();

    while (!stopFlag.loadAcquire()) {
        // If the validation queue is full, pause to let the network catch up
        if (pauseFlag.loadAcquire()) {
            msleep(50);
            continue;
        }

        QString code;
        int codeLength = 16 + (QRandomGenerator::global()->bounded(9));
        code.reserve(codeLength); // Optimize memory allocation

        for (int i = 0; i < codeLength; ++i) {
            code.append(chars.at(QRandomGenerator::global()->bounded(charsLen)));
        }

        emit codeGenerated(QString("https://discord.gift/%1").arg(code));

        // Small sleep to prevent CPU hogging
        msleep(10);
    }
}

void NitroGeneratorThread::stop() {
    stopFlag.storeRelease(true);
}

void NitroGeneratorThread::setPaused(bool paused) {
    pauseFlag.storeRelease(paused);
}

// ==========================================
// NitroGen Main Class Implementation
// ==========================================

NitroGen::NitroGen(QWidget *parent)
    : QWidget(parent),
    isGenerating(false),
    threadCount(4),
    consecutiveTimeouts(0),
    totalGenerated(0),
    validLinksFound(0)
{
    setupUI();
    setupStyling();

    networkManager = new QNetworkAccessManager(this);
    validationTimer = new QTimer(this);
    validationTimer->setInterval(50); // Validate 1 code every 50ms
    connect(validationTimer, &QTimer::timeout, this, &NitroGen::onValidationTimer);

    // Initial default save path displayed in the UI
    saveFilePath = "generated_codes.txt";
    saveInput->setText(saveFilePath);

    logToConsole("Discord Nitro Generator Console", "[SYSTEM]");
    logToConsole("Type 'help' for commands", "[SYSTEM]");
}

NitroGen::~NitroGen() {
    stopContinuousGeneration();
}

void NitroGen::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(6);

    // 1. Console Output
    consoleOutput = new QPlainTextEdit(this);
    consoleOutput->setReadOnly(true);
    consoleOutput->setMaximumBlockCount(5000);
    mainLayout->addWidget(consoleOutput);

    // 2. Success Display Area
    QHBoxLayout *linkLayout = new QHBoxLayout();
    generatedLinkLabel = new QLabel(this);
    generatedLinkLabel->hide();

    copyLinkButton = new QPushButton("Copy", this);
    copyLinkButton->hide();
    connect(copyLinkButton, &QPushButton::clicked, this, &NitroGen::onCopyClicked);

    linkLayout->addWidget(generatedLinkLabel, 1);
    linkLayout->addWidget(copyLinkButton);
    mainLayout->addLayout(linkLayout);

    // 3. Webhook Area
    QGroupBox *webhookGroup = new QGroupBox("Webhook", this);
    QHBoxLayout *hookLayout = new QHBoxLayout(webhookGroup);
    webhookInput = new QLineEdit(this);
    webhookInput->setPlaceholderText("Discord webhook URL...");

    QPushButton *sendButton = new QPushButton("Test Send", this);
    connect(sendButton, &QPushButton::clicked, this, &NitroGen::onSendWebhookClicked);

    hookLayout->addWidget(webhookInput, 1);
    hookLayout->addWidget(sendButton);
    mainLayout->addWidget(webhookGroup);

    // 4. Save Configuration
    QGroupBox *saveGroup = new QGroupBox("Save Configuration", this);
    QHBoxLayout *saveLayout = new QHBoxLayout(saveGroup);
    saveInput = new QLineEdit(this);
    browseButton = new QPushButton("Browse", this);
    connect(browseButton, &QPushButton::clicked, this, &NitroGen::onBrowseClicked);

    saveLayout->addWidget(saveInput, 1);
    saveLayout->addWidget(browseButton);
    mainLayout->addWidget(saveGroup);

    // 5. Terminal Input
    QGroupBox *terminalGroup = new QGroupBox("Terminal", this);
    QHBoxLayout *inputLayout = new QHBoxLayout(terminalGroup);
    QLabel *promptLabel = new QLabel("nitrogen@console:~$", this);
    promptLabel->setStyleSheet("color: #ff6b00; font-weight: bold;");
    commandInput = new QLineEdit(this);
    commandInput->setPlaceholderText("Enter command (gen, stop, save, save all, help)...");

    inputLayout->addWidget(promptLabel);
    inputLayout->addWidget(commandInput, 1);
    mainLayout->addWidget(terminalGroup);

    connect(commandInput, &QLineEdit::returnPressed, this, &NitroGen::handleCommand);
    commandInput->setFocus();
}

void NitroGen::handleCommand() {
    QString cmd = commandInput->text().trimmed();
    if (cmd.isEmpty()) return;

    logToConsole(QString("nitrogen@console:~$ %1").arg(cmd), "");
    processCommand(cmd);
    commandInput->clear();
}

void NitroGen::processCommand(const QString &command) {
    QString cmd = command.toLower().trimmed();

    if (cmd == "stop") {
        executeStopCommand();
    }
    else if (cmd == "clear") {
        executeClearCommand();
    }
    else if (cmd == "help") {
        executeHelpCommand();
    }
    else if (cmd == "save") {
        executeSaveCommand(); // Save only Valid
    }
    else if (cmd == "save all") {
        executeSaveAllCommand(); // Save History
    }
    else if (cmd.startsWith("gen")) {
        // Parse optional thread count: "gen 8"
        QStringList parts = cmd.split(' ', Qt::SkipEmptyParts);
        int threads = -1;
        if (parts.size() > 1) threads = parts[1].toInt();
        executeGenCommand(threads);
    }
    else {
        logToConsole(QString("Unknown command: '%1'").arg(cmd), "[ERROR]");
    }
}

// --- Core Commands ---

void NitroGen::executeHelpCommand() {
    unsigned int cores = std::thread::hardware_concurrency();

    // Clean, single-call help message
    QString helpText = R"(
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
      NITROGEN COMMAND LIST
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 ⚡ gen [n]   : Start generating (e.g., 'gen 8')
               Max suggested threads: %1
 🛑 stop      : Stop generation
 💾 save      : Save ONLY valid codes to file
 💾 save all  : Save ALL checked codes (Valid + Invalid)
 🧹 clear     : Clear console and memory
 ❓ help      : Show this menu
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
)";
    logToConsole(helpText.arg(cores), "[HELP]");
}

void NitroGen::executeGenCommand(int customThreads) {
    if (isGenerating) {
        logToConsole("Already generating. Type 'stop' first.", "[WARN]");
        return;
    }

    if (customThreads > 0) threadCount = std::clamp(customThreads, 1, 32);
    startContinuousGeneration();
}

void NitroGen::executeSaveCommand() {
    // Get path from UI. If empty, use a default file name.
    QString path = saveInput->text().trimmed();
    if (path.isEmpty()) {
        path = "valid_codes.txt"; // Default filename for valid codes
    }

    // Ensure robust path: if the path is relative (just a filename),
    // we prepend the current directory for absolute clarity in logging and file creation.
    if (!QDir::isAbsolutePath(path)) {
        path = QDir::currentPath() + QDir::separator() + path;
    }

    QMutexLocker locker(&storageMutex);
    if (validCodes.isEmpty()) {
        logToConsole("No valid codes found yet to save.", "[SAVE]");
        return;
    }

    QFile file(path);
    // Use QIODevice::Append to add to the file, not overwrite.
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        out << "\n--- SAVED VALID CODES " << getCurrentTimestamp() << " ---\n";
        for (const QString &code : validCodes) {
            out << code << "\n";
        }
        file.close();
        logToConsole(QString("Successfully saved %1 valid codes to: %2").arg(validCodes.size()).arg(path), "[SAVE]");

        // Clear saved codes after successful save
        validCodes.clear();
        logToConsole("Valid codes buffer cleared after saving.", "[SAVE]");
    } else {
        logToConsole(QString("Failed to open file for writing: %1 (Path: %2)").arg(file.errorString()).arg(path), "[ERROR]");
    }
}

void NitroGen::executeSaveAllCommand() {
    QString path = saveInput->text().trimmed();
    if (path.isEmpty()) {
        path = "history_log.txt"; // Default filename for history
    }

    // Ensure robust path
    if (!QDir::isAbsolutePath(path)) {
        path = QDir::currentPath() + QDir::separator() + path;
    }

    QMutexLocker locker(&storageMutex);
    if (historyLog.isEmpty()) {
        logToConsole("History is empty.", "[SAVE]");
        return;
    }

    QFile file(path);
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        out << "\n--- FULL HISTORY LOG " << getCurrentTimestamp() << " ---\n";
        for (const QString &line : historyLog) {
            out << line << "\n";
        }
        file.close();
        logToConsole(QString("Successfully saved %1 history entries to: %2").arg(historyLog.size()).arg(path), "[SAVE]");

        // Clear history after successful save
        historyLog.clear();
        logToConsole("History buffer cleared after saving.", "[SAVE]");
    } else {
        logToConsole(QString("Failed to open file for writing: %1 (Path: %2)").arg(file.errorString()).arg(path), "[ERROR]");
    }
}

void NitroGen::executeStopCommand() {
    if (!isGenerating) return;
    stopContinuousGeneration();
}

void NitroGen::executeClearCommand() {
    consoleOutput->clear();
    QMutexLocker locker(&storageMutex);
    validCodes.clear();
    historyLog.clear();
    generatedLinkLabel->hide();
    copyLinkButton->hide();
    logToConsole("Console and memory buffers cleared.", "[SYSTEM]");
}

// --- Generation Logic ---

void NitroGen::startContinuousGeneration() {
    isGenerating = true;
    totalGenerated = 0;
    validLinksFound = 0;
    consecutiveTimeouts = 0;

    logToConsole(QString("Starting generation with %1 threads...").arg(threadCount), "[GEN]");

    for (int i = 0; i < threadCount; ++i) {
        auto *thr = new NitroGeneratorThread(this);
        connect(thr, &NitroGeneratorThread::codeGenerated, this, &NitroGen::onCodeGenerated, Qt::QueuedConnection);
        thr->start();
        generatorThreads.append(thr);
    }
    validationTimer->start();
}

void NitroGen::stopContinuousGeneration() {
    if (!isGenerating) return;

    isGenerating = false;
    validationTimer->stop();

    logToConsole("Stopping threads...", "[STOP]");

    for (auto *thr : qAsConst(generatorThreads)) {
        thr->stop();
        thr->quit();
        thr->wait();
        delete thr;
    }
    generatorThreads.clear();

    QMutexLocker locker(&pendingMutex);
    pendingValidation.clear();

    logToConsole("Stopped.", "[STOP]");
}

void NitroGen::onCodeGenerated(const QString &link) {
    QMutexLocker locker(&pendingMutex);

    // Flow Control: Prevent RAM exhaustion
    bool shouldPause = (pendingValidation.size() > 1000);

    if (shouldPause) {
        for(auto *thr : generatorThreads) thr->setPaused(true);
    } else if (pendingValidation.size() < 500) {
        for(auto *thr : generatorThreads) thr->setPaused(false);
    }

    if (pendingValidation.size() < 2000) {
        pendingValidation.append(link);
        totalGenerated++;
    }
}

void NitroGen::onValidationTimer() {
    QString link;
    {
        QMutexLocker locker(&pendingMutex);
        if (pendingValidation.isEmpty()) return;
        link = pendingValidation.takeFirst();
    }
    validateNitroLink(link);
}

void NitroGen::validateNitroLink(const QString &link) {
    QString code = link.section('/', -1);
    QUrl url(QString("https://discord.com/api/v9/entitlements/gift-codes/%1").arg(code));

    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36");

    QNetworkReply *reply = networkManager->get(request);
    reply->setProperty("nitroLink", link);
    connect(reply, &QNetworkReply::finished, this, &NitroGen::onNetworkReply);
}

void NitroGen::onNetworkReply() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    reply->deleteLater();

    QString link = reply->property("nitroLink").toString();
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QString statusStr;

    if (reply->error() == QNetworkReply::NoError && statusCode == 200) {
        validLinksFound++;
        consecutiveTimeouts = 0;
        statusStr = "VALID";
        logToConsole(QString("VALID FOUND: %1").arg(link), "[VALID]");

        displayGeneratedLink(link);
        sendToWebhook(link);

        QMutexLocker locker(&storageMutex);
        validCodes.append(link);
    }
    else if (statusCode == 429) {
        statusStr = "RATELIMIT";
        logToConsole("RATE LIMIT HIT (429)! Pausing generation for 5 seconds.", "[WARN]");

        validationTimer->stop();
        QTimer::singleShot(5000, this, [this]() {
            if (isGenerating) {
                logToConsole("Resuming generation after 5 second backoff.", "[SYSTEM]");
                validationTimer->start();
            }
        });
    }
    else {
        statusStr = "INVALID";
        if (reply->error() == QNetworkReply::TimeoutError) consecutiveTimeouts++;
        else consecutiveTimeouts = 0;
    }

    // Auto-stop on connection death
    if (consecutiveTimeouts >= 10) {
        logToConsole("Too many timeouts. Stopping.", "[ERR]");
        stopContinuousGeneration();
    }

    // Always add to history log
    QString entry = QString("[%1] [%2] %3").arg(getCurrentTimestamp(), statusStr, link);
    QMutexLocker locker(&storageMutex);
    historyLog.append(entry);
}

// --- Utilities ---

void NitroGen::sendToWebhook(const QString &validLink) {
    if (webhookInput->text().isEmpty() || validLink.isEmpty()) return;

    QNetworkRequest request(QUrl(webhookInput->text()));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject json;
    json["content"] = QString("@everyone Found Valid: %1").arg(validLink);
    json["username"] = "NitroGen Bot";

    QNetworkReply *reply = networkManager->post(request, QJsonDocument(json).toJson());
    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
}

void NitroGen::onCopyClicked() {
    QApplication::clipboard()->setText(lastGeneratedLink);
    logToConsole("Copied to clipboard!", "[SYSTEM]");
}

void NitroGen::onSendWebhookClicked() {
    // We only test send the last found valid link, if any.
    if (!lastGeneratedLink.isEmpty()) {
        sendToWebhook(lastGeneratedLink);
        logToConsole("Attempting to send last valid link to webhook.", "[WEBHOOK]");
    } else {
        logToConsole("No valid link found yet to test send.", "[WEBHOOK]");
    }
}

void NitroGen::onBrowseClicked() {
    // Start the dialog in the current application directory, using the current input text as the suggested filename.
    QString suggestedFile = saveInput->text().trimmed().isEmpty() ? "valid_codes.txt" : saveInput->text();

    // QFileDialog::getSaveFileName(parent, caption, directory, filter)
    // We use QDir::currentPath() combined with the suggested file name for the starting point.
    QString startDir = QDir::currentPath() + QDir::separator() + suggestedFile;

    QString path = QFileDialog::getSaveFileName(this, "Select Save Location",
                                                startDir,
                                                "Text Files (*.txt)");
    if (!path.isEmpty()) {
        saveInput->setText(path);
        logToConsole(QString("Save path updated to: %1").arg(path), "[SYSTEM]");
    }
}

void NitroGen::displayGeneratedLink(const QString &link) {
    lastGeneratedLink = link;
    generatedLinkLabel->setText("VALID: " + link);
    generatedLinkLabel->show();
    copyLinkButton->show();
}

void NitroGen::logToConsole(const QString &message, const QString &prefix) {
    QString entry = QString("[%1] %2 %3")
    .arg(getCurrentTimestamp())
        .arg(prefix)
        .arg(message);
    consoleOutput->appendPlainText(entry);
    scrollToBottom();
}

void NitroGen::scrollToBottom() {
    consoleOutput->verticalScrollBar()->setValue(consoleOutput->verticalScrollBar()->maximum());
}

QString NitroGen::getCurrentTimestamp() {
    return QDateTime::currentDateTime().toString("hh:mm:ss");
}

void NitroGen::setupStyling() {
    this->setStyleSheet(R"(
        QWidget { background: #1a1a1a; color: white; font-family: Consolas, monospace; font-size: 10pt; }
        QGroupBox { border: 1px solid #ff6b00; border-radius: 5px; margin-top: 10px; font-weight: bold; }
        QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 3px; color: #ff6b00; }
        QLineEdit { background: #2a2a2a; border: 1px solid #444; padding: 4px; color: white; selection-background-color: #ff6b00; border-radius: 6px; }
        QLineEdit:focus { border: 1px solid #ff6b00; }
        QPushButton { background: #333; border: 1px solid #555; border-radius: 3px; padding: 5px; color: white; font-weight: bold; }
        QPushButton:hover { background: #444; border-color: #ff6b00; }
        QPlainTextEdit { background: #000; border: 1px solid #333; color: #0f0; font-family: Consolas; }
        QLabel { color: #ddd; }
    )");

    consoleOutput->setStyleSheet("background: #0a0a0a; border: 1px solid #333; color: #00ff00; font-family: Consolas;");
    generatedLinkLabel->setStyleSheet("color: #00ff00; font-weight: bold; border: 1px solid #00ff00; padding: 5px; background: rgba(0, 255, 0, 0.1);");
}
