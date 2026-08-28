#include "thememanager.h"
#include "systemtitlebar.h"

#include <QApplication>
#include <QFile>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QUuid>
#include <QStringList>

namespace {

QString settingsGroupCustomThemes() { return QStringLiteral("appearance/customThemes"); }
QString settingsKeyCurrentTheme() { return QStringLiteral("appearance/currentTheme"); }

// Единая точка правды для каталога, в котором хранятся файлы пользовательских
// тем (по одному INI-файлу на тему). Используем AppConfigLocation, так как
// темы - это конфигурация приложения, а не пользовательские данные проекта.
QString themesDirPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)
        + QStringLiteral("/themes");
}

// Порядок и метаданные ролей, которые пользователь может редактировать.
// displayName/description специально держим в коде (а не через tr() в
// статических данных), чтобы можно было безопасно переводить позже.
const QVector<ThemeColorRoleInfo> kEditableRoles = {
    { QPalette::Window,          QStringLiteral("windowBg"),          QObject::tr("Window background"),        QObject::tr("Main background of windows and panels") },
    { QPalette::WindowText,      QStringLiteral("windowFg"),          QObject::tr("Window text"),               QObject::tr("Primary text color") },
    { QPalette::Base,            QStringLiteral("baseBg"),            QObject::tr("Field background"),          QObject::tr("Background of text fields, lists, trees") },
    { QPalette::AlternateBase,   QStringLiteral("alternateBaseBg"),   QObject::tr("Alternate row background"),  QObject::tr("Alternating row color in lists/tables") },
    { QPalette::Text,            QStringLiteral("baseFg"),            QObject::tr("Field text"),                QObject::tr("Text color inside fields, lists, trees") },
    { QPalette::Button,          QStringLiteral("buttonBg"),          QObject::tr("Button background"),         QObject::tr("Background of buttons and comboboxes") },
    { QPalette::ButtonText,      QStringLiteral("buttonFg"),          QObject::tr("Button text"),                QObject::tr("Text color on buttons") },
    { QPalette::Highlight,       QStringLiteral("accent"),            QObject::tr("Accent / highlight"),        QObject::tr("Selection color and accent borders") },
    { QPalette::HighlightedText, QStringLiteral("accentFg"),          QObject::tr("Highlighted text"),          QObject::tr("Text color on top of the accent color") },
    { QPalette::ToolTipBase,     QStringLiteral("tooltipBg"),         QObject::tr("Tooltip background"),        QObject::tr("Background of tooltips") },
    { QPalette::ToolTipText,     QStringLiteral("tooltipFg"),         QObject::tr("Tooltip text"),              QObject::tr("Text color inside tooltips") },
    { QPalette::Mid,             QStringLiteral("borders"),           QObject::tr("Borders / separators"),      QObject::tr("Border and separator lines") },
    { QPalette::Dark,            QStringLiteral("scrollHandle"),      QObject::tr("Scrollbar handle"),          QObject::tr("Scrollbar handles and subtle surfaces") },
    { QPalette::Link,            QStringLiteral("link"),              QObject::tr("Links"),                     QObject::tr("Hyperlink color") },
};

ThemeDefinition makeDarkTheme()
{
    ThemeDefinition t;
    t.id = QStringLiteral("builtin.dark");
    t.name = QObject::tr("Dark");
    t.isBuiltin = true;
    t.iconColor = QColor("#FFFFFF");
    t.darkSystemTitleBar = true;
    t.colors = {
        { QPalette::Window,          QColor("#1E1E1E") },
        { QPalette::WindowText,      QColor("#D4D4D4") },
        { QPalette::Base,            QColor("#181818") },
        { QPalette::AlternateBase,   QColor("#1A1A1A") },
        { QPalette::Text,            QColor("#D4D4D4") },
        { QPalette::Button,          QColor("#1E1E1E") },
        { QPalette::ButtonText,      QColor("#D4D4D4") },
        { QPalette::Highlight,       QColor("#007ACC") },
        { QPalette::HighlightedText, QColor("#FFFFFF") },
        { QPalette::ToolTipBase,     QColor("#252526") },
        { QPalette::ToolTipText,     QColor("#D4D4D4") },
        { QPalette::Mid,             QColor("#3C3C3C") },
        { QPalette::Dark,            QColor("#505050") },
        { QPalette::Link,            QColor("#4EC9B0") },
    };
    return t;
}

ThemeDefinition makeLightTheme()
{
    ThemeDefinition t;
    t.id = QStringLiteral("builtin.light");
    t.name = QObject::tr("Light");
    t.isBuiltin = true;
    t.iconColor = QColor("#1E1E1E");
    t.darkSystemTitleBar = false;
    t.colors = {
        { QPalette::Window,          QColor("#F3F3F3") },
        { QPalette::WindowText,      QColor("#1E1E1E") },
        { QPalette::Base,            QColor("#FFFFFF") },
        { QPalette::AlternateBase,   QColor("#F5F5F5") },
        { QPalette::Text,            QColor("#1E1E1E") },
        { QPalette::Button,          QColor("#FFFFFF") },
        { QPalette::ButtonText,      QColor("#1E1E1E") },
        { QPalette::Highlight,       QColor("#0067C0") },
        { QPalette::HighlightedText, QColor("#FFFFFF") },
        { QPalette::ToolTipBase,     QColor("#FFFFE1") },
        { QPalette::ToolTipText,     QColor("#1E1E1E") },
        { QPalette::Mid,             QColor("#D0D0D0") },
        { QPalette::Dark,            QColor("#B0B0B0") },
        { QPalette::Link,            QColor("#0067C0") },
    };
    return t;
}

QString colorRoleKey(QPalette::ColorRole role)
{
    for (const auto& info : kEditableRoles) {
        if (info.role == role)
            return info.id;
    }
    return QString();
}

} // namespace

const QVector<ThemeColorRoleInfo>& themeEditableRoles()
{
    return kEditableRoles;
}

QPalette ThemeDefinition::toQPalette() const
{
    QPalette p = qApp ? qApp->palette() : QPalette();
    for (auto it = colors.constBegin(); it != colors.constEnd(); ++it)
        p.setColor(QPalette::Active, it.key(), it.value());
    for (auto it = colors.constBegin(); it != colors.constEnd(); ++it)
        p.setColor(QPalette::Inactive, it.key(), it.value());
    // Слегка притушенные роли для disabled-состояния, чтобы виджеты не
    // выглядели "сломанными" при отключении.
    for (auto it = colors.constBegin(); it != colors.constEnd(); ++it)
        p.setColor(QPalette::Disabled, it.key(), it.value().darker(120));

    const QColor textColor = colors.value(QPalette::Text, p.color(QPalette::Text));
    QColor placeholder = textColor;
    placeholder.setAlpha(150);
    p.setColor(QPalette::Active, QPalette::PlaceholderText, placeholder);
    p.setColor(QPalette::Inactive, QPalette::PlaceholderText, placeholder);
    p.setColor(QPalette::Disabled, QPalette::PlaceholderText, textColor.darker(140));
    return p;
}

ThemeManager& ThemeManager::instance()
{
    static ThemeManager mgr;
    return mgr;
}

ThemeManager::ThemeManager()
{
    m_builtins.append(makeDarkTheme());
    m_builtins.append(makeLightTheme());
    loadCustomThemes();

    QSettings settings;
    m_currentThemeId = settings.value(settingsKeyCurrentTheme(), m_builtins.first().id).toString();
    if (!hasTheme(m_currentThemeId))
        m_currentThemeId = m_builtins.first().id;
}

QVector<ThemeDefinition> ThemeManager::themes() const
{
    QVector<ThemeDefinition> all = m_builtins;
    all += m_customThemes;
    return all;
}

ThemeDefinition ThemeManager::theme(const QString& id) const
{
    for (const auto& t : m_builtins)
        if (t.id == id)
            return t;
    for (const auto& t : m_customThemes)
        if (t.id == id)
            return t;
    return m_builtins.first();
}

bool ThemeManager::hasTheme(const QString& id) const
{
    for (const auto& t : m_builtins)
        if (t.id == id)
            return true;
    for (const auto& t : m_customThemes)
        if (t.id == id)
            return true;
    return false;
}

QString ThemeManager::currentThemeId() const
{
    return m_currentThemeId;
}

void ThemeManager::setCurrentTheme(const QString& id)
{
    if (!hasTheme(id))
        return;

    m_currentThemeId = id;

    QSettings settings;
    settings.setValue(settingsKeyCurrentTheme(), id);

    applyThemeToApplication(theme(id));
    emit currentThemeChanged(id);
}

void ThemeManager::previewPalette(const QMap<QPalette::ColorRole, QColor>& colors)
{
    ThemeDefinition temp = theme(m_currentThemeId);
    temp.colors = colors;
    previewTheme(temp);
}

void ThemeManager::previewTheme(const ThemeDefinition& definition)
{
    applyThemeToApplication(definition);
    emit themePreviewChanged();
}

void ThemeManager::restoreCurrentTheme()
{
    applyThemeToApplication(theme(m_currentThemeId));
    emit themePreviewChanged();
}

QString ThemeManager::suggestedNewThemeName() const
{
    const QString base = tr("Custom theme");
    QStringList existingNames;
    for (const auto& t : m_customThemes)
        existingNames << t.name;

    if (!existingNames.contains(base))
        return base;

    int suffix = 2;
    while (existingNames.contains(QStringLiteral("%1 %2").arg(base).arg(suffix)))
        ++suffix;
    return QStringLiteral("%1 %2").arg(base).arg(suffix);
}

QString ThemeManager::createCustomTheme(const QString& name, const QString& baseThemeId)
{
    ThemeDefinition def;
    def.id = QStringLiteral("custom.%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
    def.name = name.trimmed().isEmpty() ? suggestedNewThemeName() : name.trimmed();
    def.isBuiltin = false;
    const ThemeDefinition base = theme(baseThemeId);
    def.colors = base.colors;
    def.iconColor = base.iconColor;
    def.darkSystemTitleBar = base.darkSystemTitleBar;

    m_customThemes.append(def);
    saveCustomThemes();
    emit themesChanged();
    return def.id;
}

bool ThemeManager::updateCustomTheme(const ThemeDefinition& definition)
{
    for (auto& t : m_customThemes) {
        if (t.id == definition.id) {
            if (t.isBuiltin)
                return false;
            t.name = definition.name;
            t.colors = definition.colors;
            t.iconColor = definition.iconColor.isValid()
                ? definition.iconColor
                : m_builtins.first().iconColor;
            t.darkSystemTitleBar = definition.darkSystemTitleBar;
            saveCustomThemes();
            emit themesChanged();
            if (m_currentThemeId == t.id)
                applyThemeToApplication(t);
            return true;
        }
    }
    return false;
}

bool ThemeManager::removeCustomTheme(const QString& id)
{
    for (int i = 0; i < m_customThemes.size(); ++i) {
        if (m_customThemes[i].id == id) {
            const QString themeFilePath =
                QDir(themesDirPath()).filePath(id + QStringLiteral(".ini"));
            m_customThemes.removeAt(i);
            QFile::remove(themeFilePath);
            saveCustomThemes();
            emit themesChanged();
            if (m_currentThemeId == id)
                setCurrentTheme(m_builtins.first().id);
            return true;
        }
    }
    return false;
}

void ThemeManager::loadCustomThemes()
{
    m_customThemes.clear();

    QDir themesDir(themesDirPath());
    if (!themesDir.exists())
        themesDir.mkpath(QStringLiteral("."));

    // One-time migration from the previous themes location (AppDataLocation)
    // to the current one (AppConfigLocation). Only runs if the new directory
    // is still empty, so it never overwrites themes the user already has
    // in the new location.
    if (themesDir.entryList({QStringLiteral("*.ini")}, QDir::Files).isEmpty()) {
        const QString oldThemesDirPath =
            QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/themes");
        QDir oldThemesDir(oldThemesDirPath);
        if (oldThemesDir.exists()) {
            const QStringList oldFiles = oldThemesDir.entryList({QStringLiteral("*.ini")}, QDir::Files);
            for (const QString& fileName : oldFiles)
                QFile::copy(oldThemesDir.filePath(fileName), themesDir.filePath(fileName));
        }
    }

    const QStringList files = themesDir.entryList({QStringLiteral("*.ini")}, QDir::Files, QDir::Name);
    for (const QString& fileName : files) {
        QSettings settings(themesDir.filePath(fileName), QSettings::IniFormat);
        ThemeDefinition def;
        def.id = settings.value(QStringLiteral("id")).toString();
        def.name = settings.value(QStringLiteral("name")).toString();
        if (def.id.isEmpty() || def.name.isEmpty())
            continue;
        def.isBuiltin = false;

        for (const auto& roleInfo : kEditableRoles) {
            const QString stored = settings.value(roleInfo.id).toString();
            const QColor color(stored);
            if (color.isValid())
                def.colors.insert(roleInfo.role, color);
        }

        const QColor iconColor(settings.value(QStringLiteral("iconColor")).toString());
        def.iconColor = iconColor.isValid() ? iconColor : m_builtins.first().iconColor;
        def.darkSystemTitleBar = settings.value(
            QStringLiteral("darkSystemTitleBar"), m_builtins.first().darkSystemTitleBar).toBool();

        // Fill newly introduced palette roles from the current Dark fallback.
        for (auto it = m_builtins.first().colors.constBegin();
             it != m_builtins.first().colors.constEnd(); ++it) {
            if (!def.colors.contains(it.key()))
                def.colors.insert(it.key(), it.value());
        }

        m_customThemes.append(def);
    }

    // One-time migration from the old QSettings array. The new source of
    // truth is now the themes directory, so themes are portable and no
    // longer mixed into the general application settings store.
    if (m_customThemes.isEmpty()) {
        QSettings legacy;
        const int count = legacy.beginReadArray(settingsGroupCustomThemes());
        for (int i = 0; i < count; ++i) {
            legacy.setArrayIndex(i);
            ThemeDefinition def;
            def.id = legacy.value(QStringLiteral("id")).toString();
            def.name = legacy.value(QStringLiteral("name")).toString();
            if (def.id.isEmpty() || def.name.isEmpty())
                continue;
            def.isBuiltin = false;
            for (const auto& roleInfo : kEditableRoles) {
                const QColor color(legacy.value(roleInfo.id).toString());
                if (color.isValid())
                    def.colors.insert(roleInfo.role, color);
            }
            const QColor iconColor(legacy.value(QStringLiteral("iconColor")).toString());
            def.iconColor = iconColor.isValid() ? iconColor : m_builtins.first().iconColor;
            if (legacy.contains(QStringLiteral("darkSystemTitleBar")))
                def.darkSystemTitleBar = legacy.value(QStringLiteral("darkSystemTitleBar")).toBool();
            else {
                const QColor oldTitleBar(legacy.value(QStringLiteral("titleBarColor")).toString());
                def.darkSystemTitleBar = oldTitleBar.isValid()
                    ? oldTitleBar.lightnessF() < 0.5
                    : m_builtins.first().darkSystemTitleBar;
            }
            for (auto it = m_builtins.first().colors.constBegin();
                 it != m_builtins.first().colors.constEnd(); ++it) {
                if (!def.colors.contains(it.key()))
                    def.colors.insert(it.key(), it.value());
            }
            m_customThemes.append(def);
        }
        legacy.endArray();
        if (!m_customThemes.isEmpty()) {
            saveCustomThemes();
            legacy.remove(settingsGroupCustomThemes());
            legacy.sync();
        }
    }
}

void ThemeManager::saveCustomThemes() const
{
    QDir themesDir(themesDirPath());
    if (!themesDir.exists())
        themesDir.mkpath(QStringLiteral("."));

    for (const auto& t : m_customThemes) {
        QSettings settings(themesDir.filePath(t.id + QStringLiteral(".ini")), QSettings::IniFormat);
        settings.clear();
        settings.setValue(QStringLiteral("id"), t.id);
        settings.setValue(QStringLiteral("name"), t.name);
        for (auto it = t.colors.constBegin(); it != t.colors.constEnd(); ++it) {
            const QString key = colorRoleKey(it.key());
            if (!key.isEmpty())
                settings.setValue(key, it.value().name(QColor::HexRgb));
        }
        settings.setValue(QStringLiteral("iconColor"), t.iconColor.name(QColor::HexRgb));
        settings.setValue(QStringLiteral("darkSystemTitleBar"), t.darkSystemTitleBar);
        settings.sync();
    }
}

void ThemeManager::applySystemTitleBar(QWidget* window) const
{
    if (!window)
        return;
    SystemTitleBar::apply(window, theme(m_currentThemeId).darkSystemTitleBar);
}

void ThemeManager::applyThemeToApplication(const ThemeDefinition& def) const
{
    if (!qApp)
        return;

    qApp->setPalette(def.toQPalette());
    qApp->setProperty("cremniyIconColor", def.iconColor);
    qApp->setProperty("cremniyDarkSystemTitleBar", def.darkSystemTitleBar);
    for (QWidget* window : qApp->topLevelWidgets())
        SystemTitleBar::apply(window, def.darkSystemTitleBar);
    qApp->setStyleSheet(regenerateStyleSheet(def));
}

QString ThemeManager::regenerateStyleSheet(const ThemeDefinition& def) const
{
    Q_UNUSED(def);
    // base.qss описывает только геометрию/размеры и не содержит цветов -
    // цвета берутся виджетами из QPalette через `palette(...)` в theme.qss.
    // Здесь просто загружаем оба ресурсных файла; сам QSS не хранит хексы
    // темы, поэтому его не нужно перегенерировать под каждую тему.
    QString result;

    QFile baseFile(":/styles/base.qss");
    if (baseFile.open(QFile::ReadOnly))
        result += QLatin1String(baseFile.readAll());

    QFile themeFile(":/styles/theme.qss");
    if (themeFile.open(QFile::ReadOnly))
        result += QLatin1Char('\n') + QLatin1String(themeFile.readAll());

    return result;
}
