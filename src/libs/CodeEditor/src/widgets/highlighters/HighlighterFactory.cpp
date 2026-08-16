#include "highlighters/HighlighterFactory.h"

#include "highlighters/AsmHighlighter.h"
#include "highlighters/MakefileHighlighter.h"
#include "highlighters/MarkdownHighlighter.h"
#include "highlighters/RuleBasedHighlighter.h"
#include "highlighters/XmlLanguageHighlighter.h"

#include "QCXXHighlighter.hpp"
#include "QJSONHighlighter.hpp"

#include <QRegularExpression>

namespace {

QVector<RegexRule> buildWordRules(const QStringList& words, const QString& formatName)
{
    QVector<RegexRule> rules;
    for (const QString& word : words)
        rules.append({QRegularExpression(QStringLiteral("\\b%1\\b").arg(QRegularExpression::escape(word))), formatName});
    return rules;
}

RuleBasedHighlighter* createCommonLanguageHighlighter(const QString& syntaxKey, QTextDocument* document)
{
    QVector<RegexRule> rules = {
        {QRegularExpression(QStringLiteral("\"[^\"\\n]*\"|'[^'\\n]*'")), QStringLiteral("String")},
        {QRegularExpression(QStringLiteral("\\b(0x[0-9A-Fa-f]+|\\d+(?:\\.\\d+)?)\\b")), QStringLiteral("Number")}
    };
    QRegularExpression commentPattern;

    if (syntaxKey == QStringLiteral("js") || syntaxKey == QStringLiteral("ts")) {
        rules += buildWordRules({QStringLiteral("break"), QStringLiteral("case"), QStringLiteral("catch"), QStringLiteral("class"), QStringLiteral("const"), QStringLiteral("continue"), QStringLiteral("default"), QStringLiteral("delete"), QStringLiteral("else"), QStringLiteral("export"), QStringLiteral("extends"), QStringLiteral("finally"), QStringLiteral("for"), QStringLiteral("from"), QStringLiteral("function"), QStringLiteral("if"), QStringLiteral("import"), QStringLiteral("in"), QStringLiteral("instanceof"), QStringLiteral("let"), QStringLiteral("new"), QStringLiteral("return"), QStringLiteral("super"), QStringLiteral("switch"), QStringLiteral("this"), QStringLiteral("throw"), QStringLiteral("try"), QStringLiteral("typeof"), QStringLiteral("var"), QStringLiteral("while"), QStringLiteral("yield"), QStringLiteral("async"), QStringLiteral("await")}, QStringLiteral("Keyword"));
        rules += buildWordRules({QStringLiteral("string"), QStringLiteral("number"), QStringLiteral("boolean"), QStringLiteral("void"), QStringLiteral("null"), QStringLiteral("undefined"), QStringLiteral("any"), QStringLiteral("unknown"), QStringLiteral("never")}, QStringLiteral("PrimitiveType"));
        commentPattern = QRegularExpression(QStringLiteral("//[^\\n]*"));
    } else if (syntaxKey == QStringLiteral("java") || syntaxKey == QStringLiteral("cs") || syntaxKey == QStringLiteral("go") || syntaxKey == QStringLiteral("php")) {
        const QStringList keywords = syntaxKey == QStringLiteral("go")
            ? QStringList{QStringLiteral("break"), QStringLiteral("case"), QStringLiteral("chan"), QStringLiteral("const"), QStringLiteral("continue"), QStringLiteral("default"), QStringLiteral("defer"), QStringLiteral("else"), QStringLiteral("fallthrough"), QStringLiteral("for"), QStringLiteral("func"), QStringLiteral("go"), QStringLiteral("goto"), QStringLiteral("if"), QStringLiteral("import"), QStringLiteral("interface"), QStringLiteral("map"), QStringLiteral("package"), QStringLiteral("range"), QStringLiteral("return"), QStringLiteral("select"), QStringLiteral("struct"), QStringLiteral("switch"), QStringLiteral("type"), QStringLiteral("var")}
            : syntaxKey == QStringLiteral("php")
                ? QStringList{QStringLiteral("class"), QStringLiteral("function"), QStringLiteral("public"), QStringLiteral("private"), QStringLiteral("protected"), QStringLiteral("if"), QStringLiteral("else"), QStringLiteral("elseif"), QStringLiteral("return"), QStringLiteral("foreach"), QStringLiteral("while"), QStringLiteral("namespace"), QStringLiteral("use"), QStringLiteral("extends"), QStringLiteral("implements"), QStringLiteral("trait"), QStringLiteral("static"), QStringLiteral("new")}
                : QStringList{QStringLiteral("abstract"), QStringLiteral("break"), QStringLiteral("case"), QStringLiteral("catch"), QStringLiteral("class"), QStringLiteral("continue"), QStringLiteral("default"), QStringLiteral("else"), QStringLiteral("enum"), QStringLiteral("extends"), QStringLiteral("finally"), QStringLiteral("for"), QStringLiteral("if"), QStringLiteral("implements"), QStringLiteral("import"), QStringLiteral("interface"), QStringLiteral("namespace"), QStringLiteral("new"), QStringLiteral("package"), QStringLiteral("private"), QStringLiteral("protected"), QStringLiteral("public"), QStringLiteral("return"), QStringLiteral("static"), QStringLiteral("switch"), QStringLiteral("this"), QStringLiteral("throw"), QStringLiteral("try"), QStringLiteral("using"), QStringLiteral("while")};
        rules += buildWordRules(keywords, QStringLiteral("Keyword"));
        rules += buildWordRules({QStringLiteral("int"), QStringLiteral("long"), QStringLiteral("short"), QStringLiteral("float"), QStringLiteral("double"), QStringLiteral("bool"), QStringLiteral("boolean"), QStringLiteral("string"), QStringLiteral("char"), QStringLiteral("byte"), QStringLiteral("void")}, QStringLiteral("PrimitiveType"));
        commentPattern = QRegularExpression(QStringLiteral("//[^\\n]*"));
    } else if (syntaxKey == QStringLiteral("sh")) {
        rules += buildWordRules({QStringLiteral("if"), QStringLiteral("then"), QStringLiteral("else"), QStringLiteral("elif"), QStringLiteral("fi"), QStringLiteral("for"), QStringLiteral("do"), QStringLiteral("done"), QStringLiteral("while"), QStringLiteral("case"), QStringLiteral("esac"), QStringLiteral("function"), QStringLiteral("in"), QStringLiteral("export"), QStringLiteral("local"), QStringLiteral("readonly")}, QStringLiteral("Keyword"));
        rules += buildWordRules({QStringLiteral("echo"), QStringLiteral("cd"), QStringLiteral("test"), QStringLiteral("printf"), QStringLiteral("source")}, QStringLiteral("Function"));
        commentPattern = QRegularExpression(QStringLiteral("#[^\\n]*"));
    } else if (syntaxKey == QStringLiteral("yaml")) {
        rules += buildWordRules({QStringLiteral("true"), QStringLiteral("false"), QStringLiteral("null"), QStringLiteral("yes"), QStringLiteral("no"), QStringLiteral("on"), QStringLiteral("off")}, QStringLiteral("Keyword"));
        rules.append({QRegularExpression(QStringLiteral("^\\s*[^:#\\n]+:(?=\\s|$)")), QStringLiteral("Function")});
        commentPattern = QRegularExpression(QStringLiteral("#[^\\n]*"));
    } else if (syntaxKey == QStringLiteral("toml")) {
        rules += buildWordRules({QStringLiteral("true"), QStringLiteral("false")}, QStringLiteral("Keyword"));
        rules.append({QRegularExpression(QStringLiteral("^\\s*\\[[^\\]]+\\]")), QStringLiteral("Type")});
        rules.append({QRegularExpression(QStringLiteral("^\\s*[A-Za-z0-9_.-]+(?=\\s*=)")), QStringLiteral("Function")});
        commentPattern = QRegularExpression(QStringLiteral("#[^\\n]*"));
    } else if (syntaxKey == QStringLiteral("ini")) {
        rules.append({QRegularExpression(QStringLiteral("^\\s*\\[[^\\]]+\\]")), QStringLiteral("Type")});
        rules.append({QRegularExpression(QStringLiteral("^\\s*[A-Za-z0-9_.-]+(?=\\s*=)")), QStringLiteral("Function")});
        commentPattern = QRegularExpression(QStringLiteral("^[;#][^\\n]*"));
    } else if (syntaxKey == QStringLiteral("sql")) {
        rules += buildWordRules({QStringLiteral("select"), QStringLiteral("from"), QStringLiteral("where"), QStringLiteral("insert"), QStringLiteral("into"), QStringLiteral("update"), QStringLiteral("delete"), QStringLiteral("join"), QStringLiteral("left"), QStringLiteral("right"), QStringLiteral("inner"), QStringLiteral("outer"), QStringLiteral("group"), QStringLiteral("by"), QStringLiteral("order"), QStringLiteral("limit"), QStringLiteral("create"), QStringLiteral("table"), QStringLiteral("alter"), QStringLiteral("drop"), QStringLiteral("index"), QStringLiteral("values"), QStringLiteral("set"), QStringLiteral("and"), QStringLiteral("or"), QStringLiteral("not"), QStringLiteral("null")}, QStringLiteral("Keyword"));
        commentPattern = QRegularExpression(QStringLiteral("--[^\\n]*"));
    } else if (syntaxKey == QStringLiteral("xml")) {
        rules.append({QRegularExpression(QStringLiteral("</?[A-Za-z_:][A-Za-z0-9:._-]*")), QStringLiteral("Keyword")});
        rules.append({QRegularExpression(QStringLiteral("\\b[A-Za-z_:][A-Za-z0-9:._-]*(?=\\=)")), QStringLiteral("Function")});
        rules.append({QRegularExpression(QStringLiteral("<!DOCTYPE[^>]*>|<\\?xml[^?]*\\?>")), QStringLiteral("Preprocessor")});
        commentPattern = QRegularExpression(QStringLiteral("<!--[^>]*-->"));
    } else if (syntaxKey == QStringLiteral("sln")) {
        rules += buildWordRules({QStringLiteral("Project"), QStringLiteral("EndProject"), QStringLiteral("Global"), QStringLiteral("EndGlobal"), QStringLiteral("GlobalSection"), QStringLiteral("EndGlobalSection")}, QStringLiteral("Keyword"));
        rules.append({QRegularExpression(QStringLiteral("\"[^\"]+\"")), QStringLiteral("String")});
        commentPattern = QRegularExpression(QStringLiteral("^#.*$"));
    }

    return new RuleBasedHighlighter(rules, commentPattern, document);
}

} // anonymous namespace

namespace HighlighterFactory {

QStyleSyntaxHighlighter* create(const QString& resourcePath,
                                 const QString& ext,
                                 QTextDocument* document)
{
    if (resourcePath == QStringLiteral(":/languages/asm.xml"))
        return new AsmHighlighter(document);

    if (resourcePath == QStringLiteral(":/languages/markdown"))
        return new MarkdownHighlighter(document);

    if (resourcePath == QStringLiteral(":/languages/json"))
        return new QJSONHighlighter(document);

    if (resourcePath == QStringLiteral(":/languages/c.xml") ||
        resourcePath == QStringLiteral(":/languages/cpp.xml"))
        return new QCXXHighlighter(document);

    if (resourcePath == QStringLiteral(":/languages/gnumake.xml"))
        return new MakefileHighlighter(document);

    if (resourcePath == QStringLiteral(":/languages/cmake.xml") ||
        resourcePath == QStringLiteral(":/languages/python.xml"))
        return new XmlLanguageHighlighter(resourcePath,
                                          QRegularExpression(QStringLiteral("#[^\\n]*")),
                                          QRegularExpression(QStringLiteral("\"[^\"\\n]*\"|'[^'\\n]*'")),
                                          QRegularExpression(QStringLiteral("\\b(0x[0-9A-Fa-f]+|\\d+(?:\\.\\d+)?)\\b")),
                                          document);

    if (resourcePath == QStringLiteral(":/languages/lua.xml"))
        return new XmlLanguageHighlighter(resourcePath,
                                          QRegularExpression(QStringLiteral("--[^\\n]*")),
                                          QRegularExpression(QStringLiteral("\"[^\"\\n]*\"|'[^'\\n]*'")),
                                          QRegularExpression(QStringLiteral("\\b(0x[0-9A-Fa-f]+|\\d+(?:\\.\\d+)?)\\b")),
                                          document);

    if (resourcePath == QStringLiteral(":/languages/glsl.xml"))
        return new XmlLanguageHighlighter(resourcePath,
                                          QRegularExpression(QStringLiteral("//[^\\n]*")),
                                          QRegularExpression(QStringLiteral("\"[^\"\\n]*\"|'[^'\\n]*'")),
                                          QRegularExpression(QStringLiteral("\\b(0x[0-9A-Fa-f]+|\\d+(?:\\.\\d+)?)\\b")),
                                          document);

    if (resourcePath == QStringLiteral(":/languages/plain"))
        return nullptr;

    // Fallback: try rule-based highlighting
    return createCommonLanguageHighlighter(ext, document);
}

} // namespace HighlighterFactory
