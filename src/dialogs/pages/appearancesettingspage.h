#ifndef APPEARANCESETTINGSPAGE_H
#define APPEARANCESETTINGSPAGE_H

#include "core/settings/settingspage.h"

#include <QEvent>

class QVBoxLayout;
class QWidget;

// Страница "Внешний вид": карточки-превью встроенных и пользовательских
// тем (клик - применить), плюс кнопки создания/дублирования/переименования/
// удаления пользовательских тем. Сама смена темы происходит немедленно
// (через ThemeManager), а не по кнопке "Применить" в диалоге настроек -
// так пользователь сразу видит результат, как в Telegram.
class AppearanceSettingsPage : public SettingsPage
{
    Q_OBJECT

public:
    explicit AppearanceSettingsPage(QWidget* parent = nullptr);

    void load() override;
    bool validate(QString* errorMessage) const override;
    void apply() override;

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void rebuildThemeList();
    void onThemeCardClicked(const QString& themeId);
    void onCreateTheme();
    void onEditTheme(const QString& themeId);
    void onDuplicateTheme(const QString& themeId);
    void onDeleteTheme(const QString& themeId);

    QWidget* m_themesContainer = nullptr;
    QVBoxLayout* m_themesLayout = nullptr;
};

#endif // APPEARANCESETTINGSPAGE_H
