#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include "TitleBar.h"

// Forward declarations
class Home;
class WhSpam;
class WhSend;
class WhCheck;
class NitroGen;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    enum PageIndex {
        HOME_PAGE,
        WHSPAM_PAGE,
        WHSEND_PAGE,
        WHCHECK_PAGE,
        NITROGEN_PAGE
    };

private:
    QStackedWidget *stackedWidget;
    TitleBar *titleBar;  // Add TitleBar member
    Home *home;
    WhSpam *whSpam;
    WhSend *whSend;
    WhCheck *whCheck;
    NitroGen *nitroGen;
    QMap<int, QString> pageTitles;

    // Bottom messages
    QWidget *statusW;
    QWidget *updatedW;
    QWidget *statusContainer;
    QLabel *statusL;
    QLabel *updatedL;
    QString *updatedDisplayL;
    QString *updatedTextValue;
    QString *statusTextValue;
    QString *statusDisplayS;
    int currentStatusIndicator;

    void onMinimizeClicked();
    void onCloseClicked();
    void onTitleBarDragged(const QPoint &delta);

private slots:
    void updateTitleBar(int index);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
};

#endif // MAINWINDOW_H
