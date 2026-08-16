#pragma once

#include <QList>
#include <QPointer>
#include <QString>
#include <QWidget>

class QVBoxLayout;
class TerminalWidget;

class TerminalSession final : public QWidget
{
    Q_OBJECT

public:
    explicit TerminalSession(const QString &workingDirectory, QWidget *parent = nullptr);

    TerminalWidget *activeTerminal() const;
    int terminalCount() const;

public slots:
    void splitActive(Qt::Orientation orientation);
    void closeActiveTerminal();
    void focusActiveTerminal();

signals:
    void titleChanged(const QString &title);
    void empty();
    void newTabRequested();

private:
    TerminalWidget *createTerminal();
    TerminalWidget *firstTerminal(QWidget *widget) const;
    void setActiveTerminal(TerminalWidget *terminal);
    void collapseSplitter(QWidget *splitterWidget);

    QString m_workingDirectory;
    QVBoxLayout *m_layout = nullptr;
    QList<QPointer<TerminalWidget>> m_terminals;
    QPointer<TerminalWidget> m_activeTerminal;
};
