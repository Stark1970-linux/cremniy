#include "paletteeditordialog.h"

#include <QColorDialog>
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

void PaletteEditorDialog::updateSwatch(QPalette::ColorRole role)
{
    QToolButton* swatch = m_swatches.value(role);
    if (!swatch)
        return;
    const QColor color = m_workingColors.value(role);
    swatch->setStyleSheet(QStringLiteral("QToolButton#paletteColorSwatch { background-color: %1; }").arg(color.name(QColor::HexRgb)));
}

void PaletteEditorDialog::applyLivePreview() const
{
    ThemeManager::instance().previewPalette(m_workingColors);
}

void PaletteEditorDialog::onSave()
{
    ThemeDefinition def;
    def.id = m_themeId;
    def.name = m_nameEdit->text().trimmed().isEmpty()
        ? ThemeManager::instance().suggestedNewThemeName()
        : m_nameEdit->text().trimmed();
    def.colors = m_workingColors;

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
