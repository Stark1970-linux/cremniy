#pragma once

#include <QString>

class QStyleSyntaxHighlighter;
class QTextDocument;

namespace HighlighterFactory {

QStyleSyntaxHighlighter* create(const QString& resourcePath,
                                 const QString& ext,
                                 QTextDocument* document);

} // namespace HighlighterFactory
