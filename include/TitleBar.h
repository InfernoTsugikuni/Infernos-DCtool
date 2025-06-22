#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QPoint>
#include <QStyle>
#include <QMouseEvent>
#include <QApplication>
#include <QCoreApplication>
#include <QFile>
#include <QPainter>
#include <QDebug>
#include <QHBoxLayout>
#include <QPainterPath>
#include <QDesktopServices>
#include <QUrl>

class TitleBar : public QWidget {
    Q_OBJECT

public:
    explicit TitleBar(QWidget *parent = nullptr);
    void setPageTitle(const QString& imagePath);

signals:
    void minimizeClicked();
    void closeClicked();
    void titleBarDragged(const QPoint &delta);
    void switchToHome();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent*) override;

private:
    QWidget *titleBar;
    QLabel *pageTitle;
    QLabel *titleLabel;
    QPushButton *minimizeButton;
    QPushButton *closeButton;
    QPushButton *githubButton;
    QPushButton *homeButton;
    QPoint offset;

    void createUI();
};

#endif // TITLEBAR_H
