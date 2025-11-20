#include "home.h"

Home::Home(QWidget *parent)
    : QWidget(parent)
{
    // Create buttons without text (text will be in images)
    btnSwitchWhSpam = new QPushButton(this);
    btnSwitchWhSend = new QPushButton(this);
    btnSwitchWhCheck = new QPushButton(this);
    btnSwitchNitroGen = new QPushButton(this);

    // Set fixed size for all buttons
    const QSize buttonSize(212, 80);
    btnSwitchWhSpam->setFixedSize(buttonSize);
    btnSwitchWhSend->setFixedSize(buttonSize);
    btnSwitchWhCheck->setFixedSize(buttonSize);
    btnSwitchNitroGen->setFixedSize(buttonSize);

    // Set pointing hand cursor to all the buttons
    btnSwitchWhSpam->setCursor(Qt::PointingHandCursor);
    btnSwitchWhSend->setCursor(Qt::PointingHandCursor);
    btnSwitchWhCheck->setCursor(Qt::PointingHandCursor);
    btnSwitchNitroGen->setCursor(Qt::PointingHandCursor);

    // Set object names for styling
    btnSwitchWhSpam->setObjectName("whSpamButton");
    btnSwitchWhSend->setObjectName("whSendButton");
    btnSwitchWhCheck->setObjectName("whCheckButton");
    btnSwitchNitroGen->setObjectName("nitroGenButton");

    // Create grid layout
    QGridLayout *gridLayout = new QGridLayout(this);
    gridLayout->setSpacing(20);
    gridLayout->setContentsMargins(40, 40, 40, 40);

    // Add buttons to grid (2x2 layout)
    gridLayout->addWidget(btnSwitchWhSpam, 0, 0);  // Row 0, Column 0
    gridLayout->addWidget(btnSwitchWhSend, 0, 1);  // Row 0, Column 1
    gridLayout->addWidget(btnSwitchWhCheck, 1, 0); // Row 1, Column 0
    gridLayout->addWidget(btnSwitchNitroGen, 1, 1);// Row 1, Column 1

    // Add stretching to center the grid
    gridLayout->setRowStretch(0, 1);
    gridLayout->setRowStretch(1, 1);
    gridLayout->setColumnStretch(0, 1);
    gridLayout->setColumnStretch(1, 1);

    // Set stylesheet with hover effects
    setStyleSheet(R"(
        QPushButton#whSpamButton {
            border: none;
            background: transparent;
            image: url(:/images/buttons/whSpam.png);
        }
        QPushButton#whSpamButton:hover {
            image: url(:/images/buttons/whSpam_hover.png);
        }
        QPushButton#whSendButton {
            border: none;
            background: transparent;
            image: url(:/images/buttons/whSend.png);
        }
        QPushButton#whSendButton:hover {
            image: url(:/images/buttons/whSend_hover.png);
        }
        QPushButton#whCheckButton {
            border: none;
            background: transparent;
            image: url(:/images/buttons/whCheck.png);
        }
        QPushButton#whCheckButton:hover {
            image: url(:/images/buttons/whCheck_hover.png);
        }
        QPushButton#nitroGenButton {
            border: none;
            background: transparent;
            image: url(:/images/buttons/nitroGen.png);
        }
        QPushButton#nitroGenButton:hover {
            image: url(:/images/buttons/nitroGen_hover.png);
        }
    )");

    // Connect signals (keep existing connections)
    connect(btnSwitchWhSpam, &QPushButton::clicked, this, &Home::switchToWhSpam);
    connect(btnSwitchWhSend, &QPushButton::clicked, this, &Home::switchToWhSend);
    connect(btnSwitchWhCheck, &QPushButton::clicked, this, &Home::switchToWhCheck);
    connect(btnSwitchNitroGen, &QPushButton::clicked, this, &Home::switchToNitroGen);
}
