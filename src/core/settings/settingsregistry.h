#ifndef SETTINGSREGISTRY_H
#define SETTINGSREGISTRY_H

#include <functional>

#include <QString>
#include <QVector>

class QWidget;
class SettingsPage;

struct SettingsPageDescriptor
{
    QString pageId;
    QString categoryId;
    std::function<QString()> categoryTitle;
    int categoryOrder = 0;
    std::function<QString()> pageTitle;
    int pageOrder = 0;
    QString ownerId;
    std::function<SettingsPage*(QWidget*)> createPage;
};

class SettingsRegistry
{
public:
    static SettingsRegistry& instance();

    bool registerPage(SettingsPageDescriptor descriptor);
    bool registerModulePage(const QString& moduleId, SettingsPageDescriptor descriptor);
    QVector<SettingsPageDescriptor> pages() const;

private:
    QVector<SettingsPageDescriptor> m_pages;
};

#endif // SETTINGSREGISTRY_H
