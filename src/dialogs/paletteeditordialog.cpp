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
    form->setVerticalSpacing(14);

    for (const ThemeColorRoleInfo& info : themeEditableRoles()) {
        auto* labelBox = new QWidget(content);
        auto* labelLayout = new QVBoxLayout(labelBox);
        labelLayout->setContentsMargins(0, 0, 0, 0);
        labelLayout->setSpacing(2);

        auto* title = new QLabel(info.displayName, labelBox);
        QFont titleFont = title->font();
        titleFont.setBold(true);
        title->setFont(titleFont);

        auto* hint = new QLabel(info.description, labelBox);
        hint->setObjectName("settingsHintLabel");
        hint->setWordWrap(true);

        labelLayout->addWidget(title);
        labelLayout->addWidget(hint);

        auto* swatch = new QToolButton(content);
        swatch->setObjectName("paletteColorSwatch");
        swatch->setFixedSize(56, 28);
        swatch->setToolTip(tr("Click to choose a color"));

        const QPalette::ColorRole role = info.role;
        connect(swatch, &QToolButton::clicked, this, [this, role]() { onPickColor(role); });

        m_swatches.insert(role, swatch);
        updateSwatch(role);

        form->addRow(labelBox, swatch);
    }

    const struct { const char* key; const char* title; const char* hint; } extraColors[] = {
        { "iconColor", QT_TR_NOOP("Icon color"), QT_TR_NOOP("Color used by file-tree and themed UI icons") },
    };

    for (const auto& info : extraColors) {
        auto* labelBox = new QWidget(content);
        auto* labelLayout = new QVBoxLayout(labelBox);
        labelLayout->setContentsMargins(0, 0, 0, 0);
        labelLayout->setSpacing(2);

        auto* title = new QLabel(tr(info.title), labelBox);
        QFont titleFont = title->font();
        titleFont.setBold(true);
        title->setFont(titleFont);

        auto* hint = new QLabel(tr(info.hint), labelBox);
        hint->setObjectName("settingsHintLabel");
        hint->setWordWrap(true);
        labelLayout->addWidget(title);
        labelLayout->addWidget(hint);

        auto* swatch = new QToolButton(content);
        swatch->setObjectName("paletteColorSwatch");
        swatch->setFixedSize(56, 28);
        swatch->setToolTip(tr("Click to choose a color"));
        const QString key = QString::fromLatin1(info.key);
        connect(swatch, &QToolButton::clicked, this, [this, key]() { onPickExtraColor(key); });
        m_extraSwatches.insert(key, swatch);
        updateExtraSwatch(key);
        form->addRow(labelBox, swatch);
    }

    auto* systemTitleBarBox = new QWidget(content);
    auto* systemTitleBarLayout = new QVBoxLayout(systemTitleBarBox);
    systemTitleBarLayout->setContentsMargins(0, 0, 0, 0);
    systemTitleBarLayout->setSpacing(2);
    auto* systemTitleBarTitle = new QLabel(tr("Dark system title bar"), systemTitleBarBox);
    QFont systemTitleBarFont = systemTitleBarTitle->font();
    systemTitleBarFont.setBold(true);
    systemTitleBarTitle->setFont(systemTitleBarFont);
    auto* systemTitleBarHint = new QLabel(
        tr("Use the dark system window title bar. When disabled, the system uses the light title bar."),
        systemTitleBarBox);
    systemTitleBarHint->setObjectName("settingsHintLabel");
    systemTitleBarHint->setWordWrap(true);
    systemTitleBarLayout->addWidget(systemTitleBarTitle);
    systemTitleBarLayout->addWidget(systemTitleBarHint);

    m_darkSystemTitleBarCheck = new QCheckBox(tr("Dark"), content);
    m_darkSystemTitleBarCheck->setChecked(m_workingTheme.darkSystemTitleBar);
    connect(m_darkSystemTitleBarCheck, &QCheckBox::toggled, this, [this](bool checked) {
        m_workingTheme.darkSystemTitleBar = checked;
        applyLivePreview();
    });
    form->addRow(systemTitleBarBox, m_darkSystemTitleBarCheck);

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
