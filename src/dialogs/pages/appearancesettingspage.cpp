#include "appearancesettingspage.h"

#include <QFrame>
#include <QFont>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QToolButton>
#include <QVBoxLayout>

#include "core/settings/settingsregistry.h"
#include "core/theme/thememanager.h"
#include "dialogs/paletteeditordialog.h"

namespace {
QString categoryTitle()
{
    return QObject::tr("Modules");
}

QString pageTitle()
{
    return QObject::tr("Appearance");
}

const bool registered = SettingsRegistry::instance().registerModulePage("appearance", {
    "modules.appearance",
    "modules",
    &categoryTitle,
    300,
    &pageTitle,
    50,
    {},
    [](QWidget* parent) { return new AppearanceSettingsPage(parent); },
    // Схема настроек этого модуля: экспорт/импорт INI и рассылка
    // SettingsNotifier узнают о ключах темы отсюда, а не от ядра -
    // это те же ключи, что использует ThemeManager под капотом.
    {
        { QStringLiteral("appearance/currentTheme"), QStringLiteral("builtin.dark") },
    }
});
}

AppearanceSettingsPage::AppearanceSettingsPage(QWidget* parent)
    : SettingsPage(parent)
{
    auto* layout = new QVBoxLayout(this);

    auto* header = new QHBoxLayout();
    auto* title = new QLabel(tr("Theme"), this);
    QFont titleFont = title->font();
    titleFont.setBold(true);
    title->setFont(titleFont);
    header->addWidget(title);
    header->addStretch(1);

    auto* createButton = new QPushButton(tr("Create theme"), this);
    connect(createButton, &QPushButton::clicked, this, &AppearanceSettingsPage::onCreateTheme);
    header->addWidget(createButton);
    layout->addLayout(header);

    auto* hint = new QLabel(tr("Click a theme to apply it. Custom themes can be edited, duplicated or removed."), this);
    hint->setObjectName("settingsHintLabel");
    hint->setWordWrap(true);
    layout->addWidget(hint);

    auto* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    m_themesContainer = new QWidget(scrollArea);
    m_themesLayout = new QVBoxLayout(m_themesContainer);
    m_themesLayout->setSpacing(8);
    m_themesLayout->addStretch(1);
    m_themesContainer->setLayout(m_themesLayout);

    scrollArea->setWidget(m_themesContainer);
    layout->addWidget(scrollArea, 1);

    connect(&ThemeManager::instance(), &ThemeManager::themesChanged, this, &AppearanceSettingsPage::rebuildThemeList);
    connect(&ThemeManager::instance(), &ThemeManager::currentThemeChanged, this, &AppearanceSettingsPage::rebuildThemeList);

    rebuildThemeList();
}

void AppearanceSettingsPage::rebuildThemeList()
{
    // Убираем всё, кроме финального stretch-а в конце списка.
    while (m_themesLayout->count() > 1) {
        QLayoutItem* item = m_themesLayout->takeAt(0);
        if (item->widget())
            item->widget()->deleteLater();
        delete item;
    }

    const QString currentId = ThemeManager::instance().currentThemeId();

    for (const ThemeDefinition& def : ThemeManager::instance().themes()) {
        auto* card = new QFrame(m_themesContainer);
        card->setObjectName("themeSwatchCard");
        card->setProperty("selected", def.id == currentId);
        card->setCursor(Qt::PointingHandCursor);

        auto* cardLayout = new QHBoxLayout(card);

        auto* preview = new QFrame(card);
        preview->setObjectName("themeSwatchPreview");
        preview->setFixedSize(40, 40);
        preview->setStyleSheet(QStringLiteral(
            "QFrame#themeSwatchPreview { background-color: %1; border: 1px solid %2; }")
            .arg(def.colors.value(QPalette::Window).name())
            .arg(def.colors.value(QPalette::Highlight).name()));
        cardLayout->addWidget(preview);

        auto* nameLabel = new QLabel(def.name, card);
        if (def.id == currentId) {
            QFont f = nameLabel->font();
            f.setBold(true);
            nameLabel->setFont(f);
        }
        cardLayout->addWidget(nameLabel);
        cardLayout->addStretch(1);

        if (def.id == currentId) {
            auto* activeLabel = new QLabel(tr("Active"), card);
            activeLabel->setObjectName("settingsHintLabel");
            cardLayout->addWidget(activeLabel);
        }

        if (!def.isBuiltin) {
            auto* menuButton = new QToolButton(card);
            menuButton->setText(QStringLiteral("\u22EE")); // ⋮
            menuButton->setAutoRaise(true);

            auto* menu = new QMenu(menuButton);
            const QString themeId = def.id;
            menu->addAction(tr("Edit"), this, [this, themeId]() { onEditTheme(themeId); });
            menu->addAction(tr("Duplicate"), this, [this, themeId]() { onDuplicateTheme(themeId); });
            menu->addSeparator();
            menu->addAction(tr("Delete"), this, [this, themeId]() { onDeleteTheme(themeId); });

            menuButton->setMenu(menu);
            menuButton->setPopupMode(QToolButton::InstantPopup);
            cardLayout->addWidget(menuButton);
        }

        const QString themeId = def.id;
        // Клик по самой карточке (но не по кнопке меню) применяет тему -
        // ловим клик через прозрачную кнопку поверх layout-а было бы
        // сложнее, поэтому используем простой eventFilter-подобный подход:
        // делаем всю карточку кликабельной через mousePressEvent обёртку.
        card->installEventFilter(this);
        card->setProperty("themeId", themeId);

        m_themesLayout->insertWidget(m_themesLayout->count() - 1, card);
    }
}

bool AppearanceSettingsPage::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        if (auto* frame = qobject_cast<QFrame*>(watched)) {
            const QString themeId = frame->property("themeId").toString();
            if (!themeId.isEmpty()) {
                onThemeCardClicked(themeId);
                return true;
            }
        }
    }
    return SettingsPage::eventFilter(watched, event);
}

void AppearanceSettingsPage::onThemeCardClicked(const QString& themeId)
{
    ThemeManager::instance().setCurrentTheme(themeId);
}

void AppearanceSettingsPage::onCreateTheme()
{
    bool ok = false;
    const QString name = QInputDialog::getText(
        this,
        tr("Create theme"),
        tr("Theme name:"),
        QLineEdit::Normal,
        ThemeManager::instance().suggestedNewThemeName(),
        &ok
    );
    if (!ok)
        return;

    const QString baseId = ThemeManager::instance().currentThemeId();
    const QString newId = ThemeManager::instance().createCustomTheme(name, baseId);

    PaletteEditorDialog editor(newId, this);
    if (editor.exec() != QDialog::Accepted) {
        // Пользователь отменил редактирование только что созданной темы -
        // не оставляем пустую тему-призрак в списке.
        ThemeManager::instance().removeCustomTheme(newId);
    }
}

void AppearanceSettingsPage::onEditTheme(const QString& themeId)
{
    PaletteEditorDialog editor(themeId, this);
    editor.exec();
}

void AppearanceSettingsPage::onDuplicateTheme(const QString& themeId)
{
    const ThemeDefinition source = ThemeManager::instance().theme(themeId);
    const QString newName = tr("%1 copy").arg(source.name);
    ThemeManager::instance().createCustomTheme(newName, themeId);
}

void AppearanceSettingsPage::onDeleteTheme(const QString& themeId)
{
    const ThemeDefinition def = ThemeManager::instance().theme(themeId);
    const auto reply = QMessageBox::question(
        this,
        tr("Delete theme"),
        tr("Delete theme \"%1\"? This cannot be undone.").arg(def.name)
    );
    if (reply == QMessageBox::Yes)
        ThemeManager::instance().removeCustomTheme(themeId);
}

void AppearanceSettingsPage::load()
{
    rebuildThemeList();
}

bool AppearanceSettingsPage::validate(QString* errorMessage) const
{
    Q_UNUSED(errorMessage);
    return true;
}

void AppearanceSettingsPage::apply()
{
    // Темы применяются немедленно по клику, отдельного шага "Apply" не требуется.
}
