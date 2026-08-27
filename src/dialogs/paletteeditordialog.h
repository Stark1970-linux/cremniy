#ifndef PALETTEEDITORDIALOG_H
#define PALETTEEDITORDIALOG_H

#include <QDialog>
#include <QMap>
#include <QPalette>

#include "core/theme/thememanager.h"

class QLineEdit;
class QToolButton;

// Редактор палитры одной темы: список ролей (Window background, Window
// text, ...), у каждой - цветной свотч-кнопка, клик открывает
// QColorDialog. Изменение сразу применяется ко всему приложению (live
// preview) через ThemeManager, а при отмене приложение
// возвращается к реально сохранённой теме.
class PaletteEditorDialog : public QDialog
{
    Q_OBJECT

public:
    // themeId - id редактируемой пользовательской темы (createCustomTheme
    // должен быть вызван заранее, чтобы у темы уже был id и базовые цвета).
    explicit PaletteEditorDialog(const QString& themeId, QWidget* parent = nullptr);

private slots:
    void onPickColor(QPalette::ColorRole role);
    void onPickExtraColor(const QString& key);
    void onSave();
    void onCancel();

private:
    void buildUi();
    void updateSwatch(QPalette::ColorRole role);
    void updateExtraSwatch(const QString& key);
    void applyLivePreview() const;

    QString m_themeId;
    QMap<QPalette::ColorRole, QColor> m_workingColors;
    QMap<QPalette::ColorRole, QToolButton*> m_swatches;
    QMap<QString, QToolButton*> m_extraSwatches;
    ThemeDefinition m_workingTheme;
    QLineEdit* m_nameEdit = nullptr;
};

#endif // PALETTEEDITORDIALOG_H
