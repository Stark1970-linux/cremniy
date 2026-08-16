#include "editmenu.h"
#include "ui/MenuBar/menufactory.h"
#include <QKeySequence>

static bool registered = []() {
  MenuFactory::instance().registerMenu("2", []() { return new EditMenu(); });
  return true;
}();

EditMenu::EditMenu() : BaseMenu(tr("Edit")) {

  m_find = new QAction(tr("Find"), this);
  m_find->setShortcut(QKeySequence::Find);
  m_replace = new QAction(tr("Replace"), this);
  m_replace->setShortcut(QKeySequence::Replace);
  m_findOpenFiles = new QAction(tr("Find in Open Files"), this);
  m_findOpenFiles->setShortcut(QKeySequence(QStringLiteral("Ctrl+Alt+F")));
  m_findInProject = new QAction(tr("Find in Project"), this);
  m_findInProject->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+F")));
  m_settings = new QAction(tr("Settings"), this);

    m_settings->setShortcuts({
        QKeySequence(Qt::CTRL | Qt::Key_Comma),
        QKeySequence("Ctrl+б"),
    });

  this->addAction(m_find);
  this->addAction(m_replace);
  this->addAction(m_findOpenFiles);
  this->addAction(m_findInProject);
  this->addSeparator();
  this->addAction(m_settings);
}

void EditMenu::setupConnections(IDEWindow *ideWind) {
  connect(m_find, &QAction::triggered, ideWind, &IDEWindow::on_Find);
  connect(m_replace, &QAction::triggered, ideWind, &IDEWindow::on_Replace);
  connect(m_findOpenFiles, &QAction::triggered, ideWind, &IDEWindow::on_FindOpenFiles);
  connect(m_findInProject, &QAction::triggered, ideWind, &IDEWindow::on_FindInProject);
  connect(m_settings, &QAction::triggered, ideWind,
          &IDEWindow::on_openSettings);
}
