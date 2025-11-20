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
#include <QTextStream>
#include <QMutex>
#include <QThread>
#include <QRandomGenerator>
#include <QWaitCondition>
#include <QScrollBar>

class NitroGeneratorThread : public QThread {
    Q_OBJECT
public:
    explicit NitroGeneratorThread(QObject *parent = nullptr);
    void run() override;
    void stop();
    void setPaused(bool paused);

signals:
    void codeGenerated(const QString &link);

private:
    QBasicAtomicInt stopFlag;
    QBasicAtomicInt pauseFlag;
};

// Main Widget Class
class NitroGen : public QWidget {
    Q_OBJECT

public:
    explicit NitroGen(QWidget *parent = nullptr);
    ~NitroGen();

private slots:
    void handleCommand();
    void onValidationTimer();
    void onNetworkReply();
    void onCodeGenerated(const QString &link);
    void onBrowseClicked();
    void onCopyClicked();
    void onSendWebhookClicked();

private:
    // --- UI Setup ---
    void setupUI();
    void setupStyling();

    // --- Command Processing ---
    void processCommand(const QString &command);
    void executeGenCommand(int customThreads = -1);
    void executeStopCommand();
    void executeClearCommand();
    void executeHelpCommand();
    void executeSaveCommand();
    void executeSaveAllCommand();

    // --- Core Logic ---
    void startContinuousGeneration();
    void stopContinuousGeneration();
    void validateNitroLink(const QString &link);
    void sendToWebhook(const QString &validLink);

    // --- Utilities ---
    void logToConsole(const QString &message, const QString &prefix = "");
    void scrollToBottom();
    void displayGeneratedLink(const QString &link);
    QString getCurrentTimestamp();

    // --- UI Members ---
    QPlainTextEdit *consoleOutput;
    QLineEdit *commandInput;
    QLabel *generatedLinkLabel;
    QPushButton *copyLinkButton;
    QLineEdit *webhookInput;
    QLineEdit *saveInput;
    QPushButton *browseButton;

    // --- Networking ---
    QNetworkAccessManager *networkManager;

    // --- State & Threading ---
    bool isGenerating;
    int threadCount;
    int consecutiveTimeouts;
    QString saveFilePath;
    QString lastGeneratedLink;

    // Queues
    QList<QString> pendingValidation;
    QMutex pendingMutex;
    QList<NitroGeneratorThread*> generatorThreads;
    QTimer *validationTimer;

    // Storage
    QList<QString> validCodes;
    QList<QString> historyLog;
    QMutex storageMutex;

    // Stats
    qint64 totalGenerated;
    qint64 validLinksFound;
};

#endif // NITROGEN_H
