#include "terminalsession.h"

#include "terminalwidget.h"

#include <QSplitter>
#include <QVBoxLayout>

TerminalSession::TerminalSession(const QString &workingDirectory, QWidget *parent)
    : QWidget(parent)
    , m_workingDirectory(workingDirectory)
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    TerminalWidget *terminal = createTerminal();
    m_layout->addWidget(terminal);
    setActiveTerminal(terminal);
}

TerminalWidget *TerminalSession::activeTerminal() const
{
    return m_activeTerminal;
}

int TerminalSession::terminalCount() const
{
    int count = 0;
    for (const QPointer<TerminalWidget> &terminal : m_terminals) {
        if (terminal)
            ++count;
    }
    return count;
}

void TerminalSession::splitActive(Qt::Orientation orientation)
{
    TerminalWidget *current = m_activeTerminal;
    if (!current)
        return;

    TerminalWidget *terminal = createTerminal();
    auto *parentSplitter = qobject_cast<QSplitter *>(current->parentWidget());
    if (parentSplitter && parentSplitter->orientation() == orientation) {
        const int index = parentSplitter->indexOf(current);
        parentSplitter->insertWidget(index + 1, terminal);
        parentSplitter->setStretchFactor(index, 1);
        parentSplitter->setStretchFactor(index + 1, 1);
    } else {
        auto *splitter = new QSplitter(orientation);
        splitter->setChildrenCollapsible(false);
        splitter->setHandleWidth(1);

        if (parentSplitter) {
            const int index = parentSplitter->indexOf(current);
            QWidget *replaced = parentSplitter->replaceWidget(index, splitter);
            splitter->addWidget(replaced);
        } else {
            m_layout->removeWidget(current);
            m_layout->addWidget(splitter);
            splitter->addWidget(current);
        }
        splitter->addWidget(terminal);
        splitter->setStretchFactor(0, 1);
        splitter->setStretchFactor(1, 1);
        splitter->setSizes({1, 1});
    }

    setActiveTerminal(terminal);
    terminal->setFocus(Qt::ShortcutFocusReason);
}

void TerminalSession::closeActiveTerminal()
{
    TerminalWidget *closing = m_activeTerminal;
    if (!closing)
        return;
    if (terminalCount() == 1) {
        emit empty();
        return;
    }

    auto *splitter = qobject_cast<QSplitter *>(closing->parentWidget());
    if (!splitter)
        return;

    const int index = splitter->indexOf(closing);
    QWidget *neighbor = index > 0 ? splitter->widget(index - 1) : splitter->widget(index + 1);
    m_activeTerminal = nullptr;
    m_terminals.removeAll(closing);
    closing->stopShell();
    closing->setParent(nullptr);
    closing->deleteLater();

    if (splitter->count() == 1) {
        QWidget *remaining = splitter->widget(0);
        collapseSplitter(splitter);
        setActiveTerminal(firstTerminal(remaining));
    } else {
        setActiveTerminal(firstTerminal(neighbor));
    }
    focusActiveTerminal();
}

void TerminalSession::focusActiveTerminal()
{
    if (m_activeTerminal)
        m_activeTerminal->setFocus(Qt::ShortcutFocusReason);
}

TerminalWidget *TerminalSession::createTerminal()
{
    auto *terminal = new TerminalWidget(nullptr, m_workingDirectory);
    m_terminals.append(terminal);

    connect(terminal, &TerminalWidget::activated, this, [this, terminal] {
        setActiveTerminal(terminal);
    });
    connect(terminal, &TerminalWidget::titleChanged, this, [this, terminal](const QString &title) {
        if (terminal == m_activeTerminal)
            emit titleChanged(title);
    });
    connect(terminal, &TerminalWidget::newTerminalRequested,
            this, &TerminalSession::newTabRequested);
    connect(terminal, &TerminalWidget::splitRequested, this, [this, terminal](Qt::Orientation orientation) {
        setActiveTerminal(terminal);
        splitActive(orientation);
    });
    connect(terminal, &TerminalWidget::closeRequested, this, [this, terminal] {
        setActiveTerminal(terminal);
        closeActiveTerminal();
    });
    connect(terminal, &QObject::destroyed, this, [this] {
        m_terminals.removeIf([](const QPointer<TerminalWidget> &item) { return item.isNull(); });
    });
    return terminal;
}

TerminalWidget *TerminalSession::firstTerminal(QWidget *widget) const
{
    if (!widget)
        return nullptr;
    if (auto *terminal = qobject_cast<TerminalWidget *>(widget))
        return terminal;
    return widget->findChild<TerminalWidget *>();
}

void TerminalSession::setActiveTerminal(TerminalWidget *terminal)
{
    if (!terminal || terminal == m_activeTerminal)
        return;
    m_activeTerminal = terminal;
    emit titleChanged(terminal->title());
}

void TerminalSession::collapseSplitter(QWidget *splitterWidget)
{
    auto *splitter = qobject_cast<QSplitter *>(splitterWidget);
    if (!splitter || splitter->count() != 1)
        return;

    QWidget *remaining = splitter->widget(0);
    remaining->setParent(nullptr);
    if (auto *parentSplitter = qobject_cast<QSplitter *>(splitter->parentWidget())) {
        const int index = parentSplitter->indexOf(splitter);
        parentSplitter->replaceWidget(index, remaining);
    } else {
        m_layout->removeWidget(splitter);
        m_layout->addWidget(remaining);
    }
    splitter->deleteLater();
}
