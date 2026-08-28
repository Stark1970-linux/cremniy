#include "paletteeditordialog.h"

#include <QColorDialog>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFont>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>

PaletteEditorDialog::PaletteEditorDialog(const QString& themeId, QWidget* parent)
    : QDialog(parent)
    , m_themeId(themeId)
{
    const ThemeDefinition def = ThemeManager::instance().theme(themeId);
    m_workingColors = def.colors;
    m_workingTheme = def;

    setObjectName(QStringLiteral("settingsDialog"));
    setWindowTitle(tr("Edit theme"));
    resize(420, 560);
    buildUi();

    if (m_nameEdit)
        m_nameEdit->setText(def.name);

    applyLivePreview();
}

void PaletteEditorDialog::buildUi()
{
    auto* rootLayout = new QVBoxLayout(this);

    auto* nameLabel = new QLabel(tr("Theme name"), this);
    m_nameEdit = new QLineEdit(this);
    rootLayout->addWidget(nameLabel);
    rootLayout->addWidget(m_nameEdit);

    auto* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    auto* content = new QWidget(scrollArea);
    auto* form = new QFormLayout(content);
    form->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    form->setVerticalSpacing(2);
    form->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
    form->setRowWrapPolicy(QFormLayout::DontWrapRows);

    // Хинт-лейбл со wordWrap помещается в отдельную строку формы,
    // растянутую на всю ширину диалога (SpanningRole). Если положить его
    // в ту же узкую label-колонку, что и заголовок, QFormLayout не всегда
    // корректно пробрасывает heightForWidth через промежуточный QWidget,
    // из-за чего многострочный хинт обрезается и наслаивается на
    // следующую строку. Отдельная широкая строка решает это надёжно.
    auto addColorRow = [this, content, form](const QString& title,
                                              const QString& description,
                                              QToolButton* swatch) {
        auto* titleLabel = new QLabel(title, content);
        QFont titleFont = titleLabel->font();
        titleFont.setBold(true);
        titleLabel->setFont(titleFont);
        form->addRow(titleLabel, swatch);

        auto* hint = new QLabel(description, content);
        hint->setObjectName("settingsHintLabel");
        hint->setWordWrap(true);
        form->addRow(hint);

        // Небольшой воздух перед следующей парой title/swatch, раз общий
        // verticalSpacing формы теперь минимальный (чтобы hint был ближе
        // к своему заголовку, а не к следующему).
        auto* spacer = new QWidget(content);
        spacer->setFixedHeight(10);
        form->addRow(spacer);
    };

    for (const ThemeColorRoleInfo& info : themeEditableRoles()) {
        auto* swatch = new QToolButton(content);
        swatch->setObjectName("paletteColorSwatch");
        swatch->setFixedSize(56, 28);
        swatch->setToolTip(tr("Click to choose a color"));

        const QPalette::ColorRole role = info.role;
        connect(swatch, &QToolButton::clicked, this, [this, role]() { onPickColor(role); });

        m_swatches.insert(role, swatch);
        updateSwatch(role);

        addColorRow(info.displayName, info.description, swatch);
    }

    const struct { const char* key; const char* title; const char* hint; } extraColors[] = {
        { "iconColor", QT_TR_NOOP("Icon color"), QT_TR_NOOP("Color used by file-tree and themed UI icons") },
    };

    for (const auto& info : extraColors) {
        auto* swatch = new QToolButton(content);
        swatch->setObjectName("paletteColorSwatch");
        swatch->setFixedSize(56, 28);
        swatch->setToolTip(tr("Click to choose a color"));
        const QString key = QString::fromLatin1(info.key);
        connect(swatch, &QToolButton::clicked, this, [this, key]() { onPickExtraColor(key); });
        m_extraSwatches.insert(key, swatch);
        updateExtraSwatch(key);

        addColorRow(tr(info.title), tr(info.hint), swatch);
    }


    auto* systemTitleBarTitle = new QLabel(tr("Dark system title bar"), content);
    QFont systemTitleBarFont = systemTitleBarTitle->font();
    systemTitleBarFont.setBold(true);
    systemTitleBarTitle->setFont(systemTitleBarFont);

    m_darkSystemTitleBarCheck = new QCheckBox(tr("Dark"), content);
    m_darkSystemTitleBarCheck->setChecked(m_workingTheme.darkSystemTitleBar);
    connect(m_darkSystemTitleBarCheck, &QCheckBox::toggled, this, [this](bool checked) {
        m_workingTheme.darkSystemTitleBar = checked;
        applyLivePreview();
    });
    form->addRow(systemTitleBarTitle, m_darkSystemTitleBarCheck);

    auto* systemTitleBarHint = new QLabel(
        tr("Use the dark system window title bar. When disabled, the system uses the light title bar."),
        content);
    systemTitleBarHint->setObjectName("settingsHintLabel");
    systemTitleBarHint->setWordWrap(true);
    form->addRow(systemTitleBarHint);

    content->setLayout(form);
    scrollArea->setWidget(content);
    rootLayout->addWidget(scrollArea, 1);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
    buttons->button(QDialogButtonBox::Save)->setText(tr("Save theme"));
    connect(buttons, &QDialogButtonBox::accepted, this, &PaletteEditorDialog::onSave);
    connect(buttons, &QDialogButtonBox::rejected, this, &PaletteEditorDialog::onCancel);
    rootLayout->addWidget(buttons);
}

void PaletteEditorDialog::onPickColor(QPalette::ColorRole role)
{
    const QColor current = m_workingColors.value(role);
    QColorDialog dialog(current, this);
    dialog.setOption(QColorDialog::DontUseNativeDialog, false);

    // Живой предпросмотр цвета прямо во время подбора в диалоге выбора
    // цвета, не только после его закрытия.
    connect(&dialog, &QColorDialog::currentColorChanged, this, [this, role](const QColor& color) {
        if (!color.isValid())
            return;
        m_workingColors[role] = color;
        updateSwatch(role);
        applyLivePreview();
    });

    if (dialog.exec() == QDialog::Accepted && dialog.selectedColor().isValid()) {
        m_workingColors[role] = dialog.selectedColor();
    } else {
        // Отмена в цветовом диалоге - откатываем именно эту роль к тому,
        // что было до открытия пикера.
        m_workingColors[role] = current;
    }
    updateSwatch(role);
    applyLivePreview();
}

void PaletteEditorDialog::onPickExtraColor(const QString& key)
{
    QColor current;
    if (key == QStringLiteral("iconColor"))
        current = m_workingTheme.iconColor;
    else
        return;

    QColorDialog dialog(current, this);
    connect(&dialog, &QColorDialog::currentColorChanged, this, [this, key](const QColor& color) {
        if (!color.isValid()) return;
        if (key == QStringLiteral("iconColor")) m_workingTheme.iconColor = color;
        updateExtraSwatch(key);
        applyLivePreview();
    });

    if (dialog.exec() == QDialog::Accepted && dialog.selectedColor().isValid()) {
        if (key == QStringLiteral("iconColor")) m_workingTheme.iconColor = dialog.selectedColor();
    } else {
        if (key == QStringLiteral("iconColor")) m_workingTheme.iconColor = current;
    }
    updateExtraSwatch(key);
    applyLivePreview();
}

void PaletteEditorDialog::updateSwatch(QPalette::ColorRole role)
{
    QToolButton* swatch = m_swatches.value(role);
    if (!swatch)
        return;
    const QColor color = m_workingColors.value(role);
    swatch->setStyleSheet(QStringLiteral("QToolButton#paletteColorSwatch { background-color: %1; }").arg(color.name(QColor::HexRgb)));
}

void PaletteEditorDialog::updateExtraSwatch(const QString& key)
{
    QToolButton* swatch = m_extraSwatches.value(key);
    if (!swatch) return;
    QColor color;
    if (key == QStringLiteral("iconColor")) color = m_workingTheme.iconColor;
    if (!color.isValid()) return;
    swatch->setStyleSheet(QStringLiteral("QToolButton#paletteColorSwatch { background-color: %1; }").arg(color.name(QColor::HexRgb)));
}

void PaletteEditorDialog::applyLivePreview() const
{
    ThemeDefinition preview = m_workingTheme;
    preview.colors = m_workingColors;
    ThemeManager::instance().previewTheme(preview);
}

void PaletteEditorDialog::onSave()
{
    ThemeDefinition def;
    def.id = m_themeId;
    def.name = m_nameEdit->text().trimmed().isEmpty()
        ? ThemeManager::instance().suggestedNewThemeName()
        : m_nameEdit->text().trimmed();
    def.colors = m_workingColors;
    def.iconColor = m_workingTheme.iconColor;
    def.darkSystemTitleBar = m_workingTheme.darkSystemTitleBar;

    ThemeManager::instance().updateCustomTheme(def);
    ThemeManager::instance().setCurrentTheme(m_themeId);
    accept();
}

void PaletteEditorDialog::onCancel()
{
    // Ничего не сохраняем и возвращаем приложение к теме, которая была
    // активна на самом деле (отменяем live preview).
    ThemeManager::instance().restoreCurrentTheme();
    reject();
}
