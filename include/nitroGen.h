#ifndef NITROGEN_H
#define NITROGEN_H

#include <QWidget>
#include <QNetworkAccessManager>
#include <QTimer>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDateTime>
#include <QClipboard>
#include <QApplication>
#include <QFile>
#include <QMutex>
#include <QThread>
#include <QRandomGenerator>

class NitroGeneratorThread : public QThread {
    Q_OBJECT
public:
    NitroGeneratorThread(QObject *parent = nullptr) : QThread(parent), stopFlag(false) {}
    void run() override {
        const QString chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
        while (!stopFlag.loadAcquire()) {
            QString code;
            int codeLength = 16 + (QRandomGenerator::global()->bounded(9));
            for (int i = 0; i < codeLength; ++i) {
                int idx = QRandomGenerator::global()->bounded(chars.length());
                code.append(chars.at(idx));
            }
            emit codeGenerated(QString("https://discord.gift/%1").arg(code));
            // Throttle generation slightly
            QThread::msleep(20);
        }
    }
    void stop() {
        stopFlag.storeRelease(true);
    }

signals:
    void codeGenerated(const QString &link);

private:
    QBasicAtomicInt stopFlag;
};

class NitroGen : public QWidget {
    Q_OBJECT
public:
    explicit NitroGen(QWidget *parent = nullptr);
    ~NitroGen();

private slots:
    void handleCommand();
    void onGenerationTimer();
    void onValidationTimer();
    void onNetworkReply();
    void onWebhookSendComplete(bool success, const QString &error);
    void onCodeGenerated(const QString &link);

private:
    void setupUI();
    void setupStyling();
    void processCommand(const QString &command);
    void executeGenCommand();
    void executeStopCommand();
    void executeClearCommand();
    void executeHelpCommand();
    void executeUnknownCommand(const QString &command);
    void startContinuousGeneration();
    void stopContinuousGeneration();
    void logToConsole(const QString &message, const QString &prefix);
    void scrollToBottom();
    QString getCurrentTimestamp();
    unsigned int hardwareThreads;
    unsigned int maxThreads;
    void displayGeneratedLink(const QString &link, bool isValid=false);
    void copyToClipboard(const QString &text);
    void sendToWebhook();
    void validateNitroLink(const QString &link);
    void saveValidLink(const QString &link);

    // UI elements
    QPlainTextEdit *consoleOutput;
    QLineEdit *commandInput;
    QLabel *generatedLinkLabel;
    QPushButton *copyLinkButton;
    QLineEdit *webhookInput;
    QPushButton *sendButton;
    QVBoxLayout *mainLayout;

    // Networking
    QNetworkAccessManager *networkManager;

    // Generation
    bool isGenerating;
    QList<QString> pendingValidation;
    QMutex pendingMutex;
    QList<NitroGeneratorThread*> generatorThreads;
    int threadCount;

    // Timers
    QTimer *validationTimer;

    // Stats
    qint64 totalGenerated;
    qint64 validLinksFound;
    QString lastGeneratedLink;
    QString saveFilePath;
};

#endif // NITROGEN_H
