#include "TitleBar.h"

TitleBar::TitleBar(QWidget* parent) : QWidget(parent)
{
    setFixedHeight(30);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NoSystemBackground);

    createUI();
}

void TitleBar::createUI()
{
    setAttribute(Qt::WA_TranslucentBackground, false);

    // Title Label
    titleLabel = new QLabel(this);
    titleLabel->setFixedSize(115, 17);
    titleLabel->setStyleSheet(
        "QLabel {"
        "    image: url(:/images/titleBar/TBLabel.png);"
        "    background: transparent;"
        "}"
        );

    // Page Title Label
    pageTitle = new QLabel(this);
    pageTitle->setFixedSize(250, 24);
    pageTitle->setAlignment(Qt::AlignCenter);

    // Control Buttons
    const QSize buttonSize(20, 20);

    // Home button (correct icon and functionality)
    homeButton = new QPushButton(this);
    homeButton->setFixedSize(buttonSize);
    homeButton->setCursor(Qt::PointingHandCursor);
    homeButton->setStyleSheet(
        "QPushButton {"
        "    image: url(:/images/titleBar/homeBut.png);"
        "    background: transparent;"
        "    border: none;"
        "}"
        "QPushButton:hover {"
        "    image: url(:/images/titleBar/homeButHover.png);"
        "}"
        );
    connect(homeButton, &QPushButton::clicked, this, &TitleBar::switchToHome);

    // GitHub button (correct icon and functionality)
    githubButton = new QPushButton(this);
    githubButton->setFixedSize(buttonSize);
    githubButton->setCursor(Qt::PointingHandCursor);
    githubButton->setStyleSheet(
        "QPushButton {"
        "    image: url(:/images/titleBar/gitBut.png);"
        "    background: transparent;"
        "    border: none;"
        "}"
        "QPushButton:hover {"
        "    image: url(:/images/titleBar/gitButHover.png);"
        "}"
        );
    connect(githubButton, &QPushButton::clicked, this, []() {
        QDesktopServices::openUrl(QUrl("https://github.com/InfernoTsugikuni/Infernos-DCtool"));
    });


    minimizeButton = new QPushButton(this);
    minimizeButton->setFixedSize(buttonSize);
    minimizeButton->setCursor(Qt::PointingHandCursor);
    minimizeButton->setStyleSheet(
        "QPushButton {"
        "    image: url(:/images/titleBar/Minimize.png);"
        "    background: transparent;"
        "    border: none;"
        "}"
        "QPushButton:hover {"
        "    image: url(:/images/titleBar/MinimizeHover.png);"
        "}"
        );
    connect(minimizeButton, &QPushButton::clicked, this, &TitleBar::minimizeClicked);

    closeButton = new QPushButton(this);
    closeButton->setFixedSize(buttonSize);
    closeButton->setCursor(Qt::PointingHandCursor);
    closeButton->setStyleSheet(
        "QPushButton {"
        "    image: url(:/images/titleBar/Close.png);"
        "    background: transparent;"
        "    border: none;"
        "}"
        "QPushButton:hover {"
        "    image: url(:/images/titleBar/CloseHover.png);"
        "}"
        );
    connect(closeButton, &QPushButton::clicked, this, &TitleBar::closeClicked);

    // Layout Configuration
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 0, 10, 0);

    layout->addWidget(titleLabel);
    layout->addWidget(githubButton);
    layout->addWidget(homeButton);
    layout->addStretch(-1);
    layout->addWidget(pageTitle);
    layout->addStretch();
    layout->addWidget(minimizeButton);
    layout->addWidget(closeButton);

    setLayout(layout);
}

void TitleBar::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        offset = event->globalPosition().toPoint();
    }
}

void TitleBar::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::LeftButton) {
        QPoint delta = event->globalPosition().toPoint() - offset;
        offset = event->globalPosition().toPoint();
        emit titleBarDragged(delta);
    }
}

void TitleBar::setPageTitle(const QString& imagePath)
{
    pageTitle->setStyleSheet(
        QString("QLabel {"
                "    image: url(%1);"
                "    background: transparent;"
                "}").arg(imagePath)
        );
}

void TitleBar::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // Style Parameters
    const int radius = 15;
    const QColor bgColor("#272727");
    const QColor borderColor("#ff6b00");
    const int borderWidth = 2;

    // Create background path
    QPainterPath path;
    const float halfBorder = borderWidth / 2.0f;

    path.moveTo(halfBorder, radius + halfBorder);
    path.quadTo(halfBorder, halfBorder, radius + halfBorder, halfBorder);
    path.lineTo(width() - radius - halfBorder, halfBorder);
    path.quadTo(width() - halfBorder, halfBorder, width() - halfBorder, radius + halfBorder);
    path.lineTo(width() - halfBorder, height() - halfBorder);
    path.lineTo(halfBorder, height() - halfBorder);
    path.closeSubpath();

    // Draw background
    p.fillPath(path, bgColor);

    // Draw border
    p.setPen(QPen(borderColor, borderWidth));
    p.drawPath(path);
}
