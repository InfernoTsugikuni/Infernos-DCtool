#ifndef FUNCTIONHELPER_H
#define FUNCTIONHELPER_H

#include <QTextEdit>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QTextCursor>
#include <QColor>

class Functions
{
public:
    static void highlightDiscordTags(QTextEdit* editor);
};

#endif // FUNCTIONHELPER_H
