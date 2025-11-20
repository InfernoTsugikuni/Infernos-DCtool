#include "functionHelper.h"

void Functions::highlightDiscordTags(QTextEdit* editor)
{
    if (!editor) return;

    // Block signals to prevent infinite loop
    bool wasBlocked = editor->blockSignals(true);

    QString text = editor->toPlainText();
    QTextCursor cursor = editor->textCursor();
    int cursorPosition = cursor.position(); // Save cursor position

    QTextCharFormat defaultFormat;
    defaultFormat.setForeground(Qt::white);
    defaultFormat.setBackground(Qt::transparent); // Use transparent instead of black

    QTextCharFormat tagFormat;
    tagFormat.setForeground(QColor("#F1F1F1"));       // Text color
    tagFormat.setBackground(QColor("#ff8d02"));       // Highlight color
    tagFormat.setFontWeight(QFont::Bold);

    QRegularExpression tagRegex(R"(@(here|everyone|[a-zA-Z0-9_]+))");

    // Clear all formatting first
    QTextCursor clearCursor(editor->document());
    clearCursor.select(QTextCursor::Document);
    clearCursor.setCharFormat(defaultFormat);

    // Apply tag highlighting
    QRegularExpressionMatchIterator i = tagRegex.globalMatch(text);
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        int start = match.capturedStart();
        int length = match.capturedLength();

        QTextCursor highlightCursor(editor->document());
        highlightCursor.setPosition(start);
        highlightCursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, length);
        highlightCursor.setCharFormat(tagFormat);
    }

    // Restore cursor position
    cursor.setPosition(cursorPosition);
    editor->setTextCursor(cursor);

    // Restore signal blocking state
    editor->blockSignals(wasBlocked);
}
