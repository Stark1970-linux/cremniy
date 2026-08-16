#include "terminalpanel.h"

#include "terminalsession.h"

#include <QHBoxLayout>
#include <QFileInfo>
#include <QIcon>
#include <QTabBar>
#include <QTabWidget>
#include <QToolButton>
#include <QVBoxLayout>

namespace {

QToolButton *createButton(const QString &iconName,
                          const QString &text,
                          const QString &toolTip,
                          QWidget *parent)
{
    auto *button = new QToolButton(parent);
    const QIcon icon(QStringLiteral(":/icons/phoicons/icons/%1.svg").arg(iconName));
    if (icon.isNull())
        button->setText(text);
    else
        button->setIcon(icon);
    button->setAutoRaise(true);
    button->setToolTip(toolTip);
    button->setAccessibleName(toolTip);
    button->setFixedSize(28, 28);
    return button;
}

QString displayTitle(const QString &title)
{
    if (title.isEmpty())
        return title;
    const QFileInfo file(title);
    if (file.isAbsolute()) {
        const QString baseName = file.completeBaseName();
        if (!baseName.isEmpty())
            return baseName;
    }
    return title;
}

} // namespace

TerminalPanel::TerminalPanel(const QString &workingDirectory, QWidget *parent)
    : QWidget(parent)
    , m_workingDirectory(workingDirectory)
{
    setObjectName(QStringLiteral("terminalPanel"));
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_tabs = new QTabWidget(this);
    m_tabs->setDocumentMode(true);
    m_tabs->setMovable(true);
    m_tabs->setTabsClosable(true);
    m_tabs->tabBar()->setElideMode(Qt::ElideRight);
    m_tabs->tabBar()->setExpanding(false);
    layout->addWidget(m_tabs);

    auto *actions = new QWidget(m_tabs);
    auto *actionsLayout = new QHBoxLayout(actions);
    actionsLayout->setContentsMargins(0, 0, 4, 0);
    actionsLayout->setSpacing(0);
    QToolButton *newButton = createButton(QStringLiteral("plus"),
                                          QStringLiteral("+"),
                                          tr("New Terminal"),
                                          actions);
    m_splitRightButton = createButton(QString(), QStringLiteral("⇥"), tr("Split Right"), actions);
    m_splitDownButton = createButton(QString(), QStringLiteral("⇣"), tr("Split Down"), actions);
    m_closeButton = createButton(QStringLiteral("trash"),
                                 QStringLiteral("×"),
                                 tr("Kill Terminal"),
                                 actions);
    actionsLayout->addWidget(newButton);
    actionsLayout->addWidget(m_splitRightButton);
    actionsLayout->addWidget(m_splitDownButton);
    actionsLayout->addWidget(m_closeButton);
    m_tabs->setCornerWidget(actions, Qt::TopRightCorner);

    connect(newButton, &QToolButton::clicked, this, &TerminalPanel::newTerminal);
    connect(m_splitRightButton, &QToolButton::clicked, this, &TerminalPanel::splitRight);
    connect(m_splitDownButton, &QToolButton::clicked, this, &TerminalPanel::splitDown);
    connect(m_closeButton, &QToolButton::clicked, this, &TerminalPanel::closeActiveTerminal);
    connect(m_tabs, &QTabWidget::tabCloseRequested, this, &TerminalPanel::closeTerminalTab);
    connect(m_tabs, &QTabWidget::currentChanged, this, [this] {
        updateActions();
        focusActiveTerminal();
    });

    newTerminal();
}

void TerminalPanel::focusActiveTerminal()
{
    if (TerminalSession *session = currentSession())
        session->focusActiveTerminal();
}

void TerminalPanel::newTerminal()
{
    auto *session = new TerminalSession(m_workingDirectory, m_tabs);
    const int index = m_tabs->addTab(session, tr("Terminal %1").arg(m_nextTerminalNumber++));
    m_tabs->setCurrentIndex(index);

    connect(session, &TerminalSession::titleChanged, this, [this, session](const QString &title) {
        const int tabIndex = m_tabs->indexOf(session);
        if (tabIndex >= 0 && !title.isEmpty()) {
            m_tabs->setTabText(tabIndex, displayTitle(title));
            m_tabs->setTabToolTip(tabIndex, title);
        }
    });
    connect(session, &TerminalSession::empty, this, [this, session] {
        closeTerminalTab(m_tabs->indexOf(session));
    });
    connect(session, &TerminalSession::newTabRequested, this, &TerminalPanel::newTerminal);
    updateActions();
    session->focusActiveTerminal();
}

void TerminalPanel::splitRight()
{
    if (TerminalSession *session = currentSession())
        session->splitActive(Qt::Horizontal);
}

void TerminalPanel::splitDown()
{
    if (TerminalSession *session = currentSession())
        session->splitActive(Qt::Vertical);
}

void TerminalPanel::closeActiveTerminal()
{
    if (TerminalSession *session = currentSession())
        session->closeActiveTerminal();
}

TerminalSession *TerminalPanel::currentSession() const
{
    return qobject_cast<TerminalSession *>(m_tabs->currentWidget());
}

void TerminalPanel::closeTerminalTab(int index)
{
    if (index < 0 || index >= m_tabs->count())
        return;
    QWidget *session = m_tabs->widget(index);
    m_tabs->removeTab(index);
    session->deleteLater();
    updateActions();
    focusActiveTerminal();
}

void TerminalPanel::updateActions()
{
    const bool hasSession = currentSession() != nullptr;
    m_splitRightButton->setEnabled(hasSession);
    m_splitDownButton->setEnabled(hasSession);
    m_closeButton->setEnabled(hasSession);
}
