#include "layoutHelper.h"
#include "functionHelper.h"

//
// -------- CUSTOM QLINEEDIT --------
//

URLInput::URLInput(QWidget* parent) : QLineEdit(parent) {
    // Set font and text style
    QFont font;
    font.setPointSize(14);
    setFont(font);

    // Set placeholder or initial text styling
    setPlaceholderText("Enter webhook URL...");
    setAlignment(Qt::AlignVCenter | Qt::AlignLeft); // Align text vertically centered and to the left

    // Add padding via stylesheet (left only)
    setStyleSheet(R"(
        QLineEdit {
            border: none;
            background: #101010;
            padding-left: 12px;
            color: white;
        }
        QLineEdit::placeholder {
            color: white;
        }
    )");
}

void URLInput::paintEvent(QPaintEvent* event) {
    QLineEdit::paintEvent(event); // Draw default content

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QPen pen(QColor(255, 107, 0)); // Orange border
    pen.setWidth(2);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);

    QRectF rect = this->rect();
    rect.adjust(1, 1, -1, -1); // Avoid clipping

    int radius = 8;

    // Draw border with rounded corners on left, sharp on right
    QPainterPath path;
    path.moveTo(rect.topRight());
    path.lineTo(rect.topLeft().x() + radius, rect.top());
    path.quadTo(rect.topLeft(), QPointF(rect.left(), rect.top() + radius));
    path.lineTo(rect.bottomLeft().x(), rect.bottom() - radius);
    path.quadTo(rect.bottomLeft(), QPointF(rect.left() + radius, rect.bottom()));
    path.lineTo(rect.bottomRight());

    path.lineTo(rect.topRight());

    painter.drawPath(path);
}

//
// -------- URL INPUTS (WEBHOOK SENDER AND SPAMMER) --------
//

// Webhook URL Input
WebhookUrlInput::WebhookUrlInput(QWidget* parent) : QLineEdit(parent) {
    setFixedSize(531, 31);
    setPlaceholderText("Webhook URL");
    setStyleSheet(
        "background: transparent;"
        "border: 1px solid #ff6b00;"
        "border-radius: 5px;"
        "padding: 8px;"
        "color: white;"
        "font-size: 12px;"
        "QLineEdit::placeholder { color: #999999; }"
        );
}

// Username Input
UsernameInput::UsernameInput(QWidget* parent) : QLineEdit(parent) {
    setFixedSize(531, 31);
    setPlaceholderText("Username");
    setStyleSheet(
        "background: transparent;"
        "border: 1px solid #ff6b00;"
        "border-radius: 5px;"
        "padding: 8px;"
        "color: white;"
        "font-size: 12px;"
        "QLineEdit::placeholder { color: #999999; }"
        );
}

// Avatar URL Input
AvatarUrlInput::AvatarUrlInput(QWidget* parent) : QLineEdit(parent) {
    setFixedSize(531, 31);
    setPlaceholderText("Avatar URL");
    setStyleSheet(
        "background: transparent;"
        "border: 1px solid #ff6b00;"
        "border-radius: 5px;"
        "padding: 8px;"
        "color: white;"
        "font-size: 12px;"
        "QLineEdit::placeholder { color: #999999; }"
        );
}

// Content Input (Scrollable)
ContentInput::ContentInput(QWidget* parent) : QTextEdit(parent) {
    setFixedSize(531, 98);
    setPlaceholderText("Content");
    setStyleSheet(
        "background: transparent;"
        "border: 1px solid #ff6b00;"
        "border-radius: 5px;"
        "padding: 8px;"
        "color: white;"
        "font-size: 12px;"
        "QTextEdit::placeholder { color: #999999; }"
        );
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded); // Enable scrolling

    connect(this, &QTextEdit::textChanged, this, &ContentInput::onTextChanged);
}

void ContentInput::onTextChanged()
{
    Functions::highlightDiscordTags(this);
}
