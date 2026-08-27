#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QColor>
#include <QMap>
#include <QObject>
#include <QPalette>
#include <QString>
#include <QVector>

// Одна редактируемая роль цвета темы. Мы не выставляем наружу все ~20 ролей
// QPalette, а только те, что реально влияют на вид приложения (Fusion +
// наш QSS используют именно их). Ключ - QPalette::ColorRole, но хранится
// как int, чтобы не тащить QPalette в заголовки, которым это не нужно.
struct ThemeColorRoleInfo
{
    QPalette::ColorRole role;
    QString id;            // стабильный id для сериализации, напр. "windowBg"
    QString displayName;    // человекочитаемое имя для UI
    QString description;    // короткая подсказка ("white: fallback for background")
};

// Список редактируемых ролей темы в порядке отображения в редакторе.
const QVector<ThemeColorRoleInfo>& themeEditableRoles();

struct ThemeDefinition
{
    QString id;                       // стабильный id ("builtin.dark", "custom.<uuid>")
    QString name;                     // отображаемое имя
    bool isBuiltin = false;           // встроенные темы нельзя удалить/переименовать
    QMap<QPalette::ColorRole, QColor> colors;
    QColor iconColor;
    QColor titleBarColor;
    QMap<QString, QColor> projectIconColors;

    QPalette toQPalette() const;
};

class ThemeManager : public QObject
{
    Q_OBJECT

public:
    static ThemeManager& instance();

    // Все доступные темы: встроенные + пользовательские, в стабильном порядке.
    QVector<ThemeDefinition> themes() const;

    ThemeDefinition theme(const QString& id) const;
    bool hasTheme(const QString& id) const;

    QString currentThemeId() const;
    // Применяет тему к QApplication (палитра + перегенерированный QSS) и
    // запоминает её как текущую в настройках.
    void setCurrentTheme(const QString& id);

    // Применяет палитру немедленно, не сохраняя выбор (используется для
    // live-превью в редакторе палитры).
    void previewPalette(const QMap<QPalette::ColorRole, QColor>& colors);
    // Применяет полную тему без сохранения (используется live-preview).
    void previewTheme(const ThemeDefinition& definition);
    // Возвращает приложение к сохранённой текущей теме (отмена превью).
    void restoreCurrentTheme();

    // Создаёт новую пользовательскую тему (копию baseThemeId) и возвращает её id.
    QString createCustomTheme(const QString& name, const QString& baseThemeId);
    bool updateCustomTheme(const ThemeDefinition& definition);
    bool removeCustomTheme(const QString& id);
    // Уникальное имя по умолчанию вида "Custom theme", "Custom theme 2", ...
    QString suggestedNewThemeName() const;

signals:
    void themesChanged();
    void currentThemeChanged(const QString& id);
    void themePreviewChanged();

private:
    ThemeManager();

    void loadCustomThemes();
    void saveCustomThemes() const;
    void applyThemeToApplication(const ThemeDefinition& def) const;
    QString regenerateStyleSheet(const ThemeDefinition& def) const;

    QVector<ThemeDefinition> m_builtins;
    QVector<ThemeDefinition> m_customThemes;
    QString m_currentThemeId;
};

#endif // THEMEMANAGER_H
