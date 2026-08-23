#include "appsettings.h"

#include <QSettings>
#include <QFileInfo>

static QSettings &settings()
{
    static QSettings s;
    return s;
}

QString AppSettings::keyLanguage() { return "application/language"; }
QString AppSettings::keyDisasmBackend() { return "modules/disassembler/backend"; }
QString AppSettings::keyObjdumpPath()   { return "modules/disassembler/objdumpPath"; }
QString AppSettings::keyRadare2Path()   { return "modules/disassembler/radare2Path"; }
QString AppSettings::keyInsnLimitPerSection() { return "modules/disassembler/insnLimitPerSection"; }
QString AppSettings::keyRadare2AnalysisLevel() { return "modules/disassembler/radare2/analysisLevel"; }
QString AppSettings::keyAsmSyntax() { return "modules/disassembler/asmSyntax"; }
QString AppSettings::keyRadare2PreCommands() { return "modules/disassembler/radare2/preCommands"; }
QString AppSettings::keyExcludedPatterns() { return "workspace/files/excludedPatterns"; }
QString AppSettings::keyGitBlameEnabled() { return "modules/codeEditor/gitBlameEnabled"; }
QString AppSettings::keyGitBlameColor() { return "modules/codeEditor/gitBlameColor"; }
QString AppSettings::keyGitBlamePadding() { return "modules/codeEditor/gitBlamePadding"; }

QString AppSettings::language()
{
    const QString stored = settings().value(keyLanguage()).toString().trimmed();
    return stored.isEmpty() ? QStringLiteral("en") : stored;
}

void AppSettings::setLanguage(const QString& locale)
{
    settings().setValue(keyLanguage(), locale.trimmed());
}

AppSettings::DisasmBackend AppSettings::disasmBackend()
{
    const int v = settings().value(keyDisasmBackend(), static_cast<int>(DisasmBackend::Objdump)).toInt();
    if (v == static_cast<int>(DisasmBackend::Radare2)) return DisasmBackend::Radare2;
    return DisasmBackend::Objdump;
}

void AppSettings::setDisasmBackend(DisasmBackend backend)
{
    settings().setValue(keyDisasmBackend(), static_cast<int>(backend));
}

QString AppSettings::objdumpPath()
{
    return settings().value(keyObjdumpPath()).toString().trimmed();
}

void AppSettings::setObjdumpPath(const QString &path)
{
    settings().setValue(keyObjdumpPath(), path.trimmed());
}

QString AppSettings::radare2Path()
{
    return settings().value(keyRadare2Path()).toString().trimmed();
}

void AppSettings::setRadare2Path(const QString &path)
{
    settings().setValue(keyRadare2Path(), path.trimmed());
}

int AppSettings::disasmInsnLimitPerSection()
{
    const int v = settings().value(keyInsnLimitPerSection(), 4000).toInt();
    if (v < 50) return 50;
    if (v > 200000) return 200000;
    return v;
}

void AppSettings::setDisasmInsnLimitPerSection(int limit)
{
    settings().setValue(keyInsnLimitPerSection(), limit);
}

AppSettings::Radare2AnalysisLevel AppSettings::radare2AnalysisLevel()
{
    const int v = settings().value(keyRadare2AnalysisLevel(), static_cast<int>(Radare2AnalysisLevel::None)).toInt();
    if (v == static_cast<int>(Radare2AnalysisLevel::Aaa)) return Radare2AnalysisLevel::Aaa;
    if (v == static_cast<int>(Radare2AnalysisLevel::Aa)) return Radare2AnalysisLevel::Aa;
    return Radare2AnalysisLevel::None;
}

void AppSettings::setRadare2AnalysisLevel(Radare2AnalysisLevel lvl)
{
    settings().setValue(keyRadare2AnalysisLevel(), static_cast<int>(lvl));
}

AppSettings::AsmSyntax AppSettings::asmSyntax()
{
    const int v = settings().value(keyAsmSyntax(), static_cast<int>(AsmSyntax::Intel)).toInt();
    if (v == static_cast<int>(AsmSyntax::Att)) return AsmSyntax::Att;
    return AsmSyntax::Intel;
}

void AppSettings::setAsmSyntax(AsmSyntax syntax)
{
    settings().setValue(keyAsmSyntax(), static_cast<int>(syntax));
}

QString AppSettings::radare2PreCommands()
{
    return settings().value(keyRadare2PreCommands()).toString().trimmed();
}

void AppSettings::setRadare2PreCommands(const QString &cmds)
{
    settings().setValue(keyRadare2PreCommands(), cmds.trimmed());
}

QStringList AppSettings::excludedPatterns()
{
    const QString raw = settings().value(keyExcludedPatterns()).toString();
    if (raw.trimmed().isEmpty())
        return {};
    QStringList list = raw.split(';', Qt::SkipEmptyParts);
    for (QString &p : list)
        p = p.trimmed();
    list.removeAll(QString());
    return list;
}

void AppSettings::setExcludedPatterns(const QStringList &patterns)
{
    settings().setValue(keyExcludedPatterns(), patterns.join(';'));
    emit SettingsNotifier::instance()->excludedPatternsChanged();
}

bool AppSettings::gitBlameEnabled()
{
    return settings().value(keyGitBlameEnabled(), false).toBool();
}

void AppSettings::setGitBlameEnabled(bool enabled)
{
    settings().setValue(keyGitBlameEnabled(), enabled);
    emit SettingsNotifier::instance()->gitBlameEnabledChanged(enabled);
}

QString AppSettings::gitBlameColor()
{
    return settings().value(keyGitBlameColor(), "#6D6552").toString();
}

void AppSettings::setGitBlameColor(const QString &color)
{
    settings().setValue(keyGitBlameColor(), color);
    emit SettingsNotifier::instance()->gitBlameColorChanged(color);
}

int AppSettings::gitBlamePadding()
{
    return settings().value(keyGitBlamePadding(), 6).toInt();
}

void AppSettings::setGitBlamePadding(int padding)
{
    settings().setValue(keyGitBlamePadding(), padding);
    emit SettingsNotifier::instance()->gitBlamePaddingChanged(padding);
}

SettingsNotifier *SettingsNotifier::instance()
{
    static SettingsNotifier s;
    return &s;
}

bool AppSettings::exportToIni(const QString &filePath, QString *error)
{
    const QFileInfo fi(filePath);
    if (fi.filePath().trimmed().isEmpty()) {
        if (error) *error = QObject::tr("Empty file path");
        return false;
    }

    QSettings out(fi.filePath(), QSettings::IniFormat);
    out.clear();
    const QStringList allowed = {
        keyLanguage(), keyDisasmBackend(), keyObjdumpPath(), keyRadare2Path(),
        keyInsnLimitPerSection(), keyRadare2AnalysisLevel(), keyAsmSyntax(),
        keyRadare2PreCommands(), keyExcludedPatterns(), keyGitBlameEnabled(),
        keyGitBlameColor(), keyGitBlamePadding(),
    };
    for (const QString& key : allowed) {
        if (settings().contains(key))
            out.setValue(key, settings().value(key));
    }
    out.sync();
    if (out.status() != QSettings::NoError) {
        if (error) *error = QObject::tr("Failed to write INI file");
        return false;
    }
    return true;
}

bool AppSettings::importFromIni(const QString &filePath, QString *error)
{
    const QFileInfo fi(filePath);
    if (!fi.exists() || !fi.isFile()) {
        if (error) *error = QObject::tr("File does not exist");
        return false;
    }

    QSettings in(fi.filePath(), QSettings::IniFormat);
    if (in.status() != QSettings::NoError) {
        if (error) *error = QObject::tr("Failed to read INI file");
        return false;
    }

    // Only import known keys (so random settings won't pollute).
    const QStringList allowed = {
        keyLanguage(),
        keyDisasmBackend(),
        keyObjdumpPath(),
        keyRadare2Path(),
        keyInsnLimitPerSection(),
        keyRadare2AnalysisLevel(),
        keyAsmSyntax(),
        keyRadare2PreCommands(),
        keyExcludedPatterns(),
        keyGitBlameEnabled(),
        keyGitBlameColor(),
        keyGitBlamePadding(),
    };

    bool disassemblerChanged = false;
    bool excludedPatternsChanged = false;
    bool gitBlameEnabledChanged = false;
    bool gitBlameColorChanged = false;
    bool gitBlamePaddingChanged = false;
    for (const QString &k : allowed) {
        if (in.contains(k))
            settings().setValue(k, in.value(k));
        if (in.contains(k) && (k == keyDisasmBackend() || k == keyObjdumpPath()
            || k == keyRadare2Path() || k == keyInsnLimitPerSection()
            || k == keyRadare2AnalysisLevel() || k == keyAsmSyntax()
            || k == keyRadare2PreCommands()))
            disassemblerChanged = true;
        excludedPatternsChanged = excludedPatternsChanged || (in.contains(k) && k == keyExcludedPatterns());
        gitBlameEnabledChanged = gitBlameEnabledChanged || (in.contains(k) && k == keyGitBlameEnabled());
        gitBlameColorChanged = gitBlameColorChanged || (in.contains(k) && k == keyGitBlameColor());
        gitBlamePaddingChanged = gitBlamePaddingChanged || (in.contains(k) && k == keyGitBlamePadding());
    }
    settings().sync();
    if (settings().status() != QSettings::NoError) {
        if (error) *error = QObject::tr("Failed to apply settings");
        return false;
    }
    if (excludedPatternsChanged)
        emit SettingsNotifier::instance()->excludedPatternsChanged();
    if (gitBlameEnabledChanged)
        emit SettingsNotifier::instance()->gitBlameEnabledChanged(gitBlameEnabled());
    if (gitBlameColorChanged)
        emit SettingsNotifier::instance()->gitBlameColorChanged(gitBlameColor());
    if (gitBlamePaddingChanged)
        emit SettingsNotifier::instance()->gitBlamePaddingChanged(gitBlamePadding());
    if (disassemblerChanged)
        emit SettingsNotifier::instance()->disassemblerSettingsChanged();
    return true;
}
