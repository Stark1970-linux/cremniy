#pragma once

#include <QString>
#include <QWidget>

class QTabWidget;
class QToolButton;
class TerminalSession;

class TerminalPanel final : public QWidget
{
    Q_OBJECT

public:
    explicit TerminalPanel(const QString &workingDirectory, QWidget *parent = nullptr);

    void focusActiveTerminal();

public slots:
    void newTerminal();
    void splitRight();
    void splitDown();
    void closeActiveTerminal();

private:
    TerminalSession *currentSession() const;
    void closeTerminalTab(int index);
    void updateActions();

    QString m_workingDirectory;
    QTabWidget *m_tabs = nullptr;
    QToolButton *m_splitRightButton = nullptr;
    QToolButton *m_splitDownButton = nullptr;
    QToolButton *m_closeButton = nullptr;
    int m_nextTerminalNumber = 1;
};
