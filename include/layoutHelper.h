#ifndef LAYOUTHELPER_H
#define LAYOUTHELPER_H

#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QPainter>
#include <QPainterPath>

class URLInput : public QLineEdit
{
    Q_OBJECT

public:
    explicit URLInput(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
};

class WebhookUrlInput : public QLineEdit
{
    Q_OBJECT

public:
    explicit WebhookUrlInput(QWidget* parent = nullptr);

    // Getter methods
    QString getUrl() const { return text(); }
};

class AvatarUrlInput : public QLineEdit
{
    Q_OBJECT

public:
    explicit AvatarUrlInput(QWidget* parent = nullptr);

    // Getter method
    QString getUrl() const { return text(); }
};

class ContentInput : public QTextEdit
{
    Q_OBJECT

public:
    explicit ContentInput(QWidget* parent = nullptr);

    // Getter method
    QString getContent() const { return toPlainText(); }

private slots:
    void onTextChanged(); // Slot for text changes
};

class UsernameInput : public QLineEdit
{
    Q_OBJECT

public:
    explicit UsernameInput(QWidget* parent = nullptr);

    // Getter methods
    QString getUrl() const { return text(); }
};

#endif // LAYOUTHELPER_H
