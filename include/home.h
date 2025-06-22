#ifndef HOME_H
#define HOME_H

#include <QWidget>
#include <QPushButton>
#include <QGridLayout>
#include <QLabel>

class Home : public QWidget
{
    Q_OBJECT

public:
    explicit Home(QWidget *parent = nullptr);

signals:
    void switchToWhSpam();  // Change to webhook spammer
    void switchToWhSend(); // Change to webhook sender
    void switchToWhCheck(); // Change to webhook checker
    void switchToNitroGen(); // Change to Nitro generator

private:
    QPushButton *btnSwitchWhSpam;
    QPushButton *btnSwitchWhSend;
    QPushButton *btnSwitchWhCheck;
    QPushButton *btnSwitchNitroGen;
};

#endif // HOME_H
