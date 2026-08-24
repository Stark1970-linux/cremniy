#include "logview.h"


logView::logView() {
    setReadOnly(true);
    setMaximumBlockCount(1000);

}

void logView::onErrorReceived(const QString &log){
    QTextCharFormat format;
    format.setForeground(Qt::red);
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End);
    cursor.insertText(log + "\n", format);
    setTextCursor(cursor);
}

void logView::onOutputReceived(const QString &log){
    appendPlainText(log);
}