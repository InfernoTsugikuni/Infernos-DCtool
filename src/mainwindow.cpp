#include "mainwindow.h"
#include "home.h"
#include "whSpam.h"
#include "whSend.h"
#include "whCheck.h"
#include "nitroGen.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(600, 450);

    // Create title bar
    titleBar = new TitleBar(this);
    connect(titleBar, &TitleBar::minimizeClicked, this, &MainWindow::onMinimizeClicked);
    connect(titleBar, &TitleBar::closeClicked, this, &MainWindow::onCloseClicked);
    connect(titleBar, &TitleBar::titleBarDragged, this, &MainWindow::onTitleBarDragged);

    // Initialize title images map
    pageTitles = {
        {HOME_PAGE, ":/images/titles/homePage.png"},
        {WHSPAM_PAGE, ":/images/titles/webhook_spammerPage.png"},
        {WHSEND_PAGE, ":/images/titles/webhook_messagePage.png"},
        {WHCHECK_PAGE, ":/images/titles/webhook_checkerPage.png"},
        {NITROGEN_PAGE, ":/images/titles/nitro_generatorPage.png"}
    };

    // Create stacked widget container
    stackedWidget = new QStackedWidget(this);
    stackedWidget->setCurrentIndex(HOME_PAGE);
    updateTitleBar(HOME_PAGE);

    // ---------------------------------------- STATUS BAR START ----------------------------------------\\

    QString statusTextValue; // This will hold the actual status string like "Operational"
    QString updatedTextValue = "6. 5. 2025";

    int currentStatusIndicator = 1; // 1 = Working | 2 = May be broken | 3 = Broken

    // Determine the status text based on currentStatusIndicator
    if (currentStatusIndicator == 1) {
        statusTextValue = "Operational";
    } else if (currentStatusIndicator == 2) {
        statusTextValue = "Might not work as expected";
    } else if (currentStatusIndicator == 3) {
        statusTextValue = "Not working";
    }

    // Now that statusTextValue and updatedTextValue have their values, create the full display strings
    QString statusDisplayString = QString("Status: %1").arg(statusTextValue);
    QString updatedDisplayString = QString("Last updated on: %1").arg(updatedTextValue);

    statusL = new QLabel(statusDisplayString, this);
    updatedL = new QLabel(updatedDisplayString, this);

    // Style for the 'updated' label (always orange)
    updatedL->setStyleSheet("color: #ff6b00; font-weight: bold;");

    // Apply specific styles for the 'status' label based on currentStatusIndicator
    if (currentStatusIndicator == 1) {
        statusL->setStyleSheet("color: #2CBE19; font-weight: bold;");
    } else if (currentStatusIndicator == 2) {
        statusL->setStyleSheet("color: #FFCC00; font-weight: bold;");
    } else if (currentStatusIndicator == 3) {
        statusL->setStyleSheet("color: #FF0000; font-weight: bold;");
    }

    // Create a horizontal container for the status bar
    QHBoxLayout *statusBarLayout = new QHBoxLayout();
    statusBarLayout->setContentsMargins(10, 5, 10, 10);  // Left, Top, Right, Bottom

    // Add status label
    // statusBarLayout->addWidget(statusW); // Removed
    statusBarLayout->addWidget(statusL);
    statusBarLayout->addSpacing(20);  // Adjusted spacing, was 10. Consider if this is desired.

    // Add update label
    // statusBarLayout->addWidget(updatedW); // Removed
    statusBarLayout->addWidget(updatedL);
    statusBarLayout->addStretch();  // Push everything to the left

    statusContainer = new QWidget(this); // Member variable assignment
    statusContainer->setLayout(statusBarLayout);

    // ---------------------------------------- STATUS BAR END ----------------------------------------\\

    // Modify your existing main layout setup
    QWidget *centralWidget = new QWidget(this);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->addWidget(titleBar);
    mainLayout->addWidget(stackedWidget);
    mainLayout->addWidget(statusContainer);  // Add status bar at bottom
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    setCentralWidget(centralWidget);

    // Create and add pages
    home = new Home();
    whSpam = new WhSpam();
    whSend = new WhSend();
    whCheck = new WhCheck();
    nitroGen = new NitroGen();

    stackedWidget->addWidget(home);
    stackedWidget->addWidget(whSpam);
    stackedWidget->addWidget(whSend);
    stackedWidget->addWidget(whCheck);
    stackedWidget->addWidget(nitroGen);

    // Connect HOME PAGE signals
    connect(home, &Home::switchToWhSpam, [=]() {
        stackedWidget->setCurrentIndex(MainWindow::WHSPAM_PAGE);
    });
    connect(home, &Home::switchToWhSend, [=]() {
        stackedWidget->setCurrentIndex(MainWindow::WHSEND_PAGE);
    });
    connect(home, &Home::switchToWhCheck, [=]() {
        stackedWidget->setCurrentIndex(MainWindow::WHCHECK_PAGE);
    });
    connect(home, &Home::switchToNitroGen, [=]() {
        stackedWidget->setCurrentIndex(MainWindow::NITROGEN_PAGE);
    });

    // Connect RETURN TO HOME signals
    connect(titleBar, &TitleBar::switchToHome, [=]() {
        stackedWidget->setCurrentIndex(MainWindow::HOME_PAGE);
    });

    // Connect stacked widget changes to title updates
    connect(stackedWidget, &QStackedWidget::currentChanged,
            this, &MainWindow::updateTitleBar);
}

MainWindow::~MainWindow() {}

// Title Bar methods (unchanged)
void MainWindow::onMinimizeClicked() {
    showMinimized();
}

void MainWindow::onCloseClicked() {
    close();
}

void MainWindow::onTitleBarDragged(const QPoint &delta) {
    move(pos() + delta);
}

void MainWindow::updateTitleBar(int index) {
    if(pageTitles.contains(index)) {
        titleBar->setPageTitle(pageTitles.value(index));
    }
}

// Paint and Resize events (unchanged from your code)
void MainWindow::resizeEvent(QResizeEvent* event) {
    const int radius = 15;
    QPixmap pixmap(size());
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(Qt::black); // This brush is for the mask
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(rect(), radius, radius);
    // painter.end(); // QPainter destructor will end painting

    setMask(pixmap.mask()); // Apply the mask for rounded corners
    QMainWindow::resizeEvent(event);
}

void MainWindow::paintEvent(QPaintEvent* /*event*/) {
    const int radius = 15;
    const int borderWidth = 2;
    const QColor bgColor("#1A1A1A");
    const QColor borderColor("#ff6b00");

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Define the rectangle for drawing, adjusted for border
    QRectF fullRect = rect().adjusted(
        borderWidth / 2.0,
        borderWidth / 2.0,
        -borderWidth / 2.0,
        -borderWidth / 2.0
        );

    QPainterPath fullPath;
    fullPath.addRoundedRect(fullRect, radius, radius);

    // Fill background
    painter.fillPath(fullPath, bgColor);

    // Draw border
    QPen borderPen(borderColor, borderWidth);
    borderPen.setJoinStyle(Qt::RoundJoin); // Improves corner appearance
    painter.setPen(borderPen);
    painter.drawPath(fullPath);
}
