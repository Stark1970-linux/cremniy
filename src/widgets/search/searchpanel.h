#pragma once

#include "core/search/searchengine.h"

#include <QWidget>

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QStackedWidget;
class QTimer;
class QToolButton;
class QTreeWidget;
class QTreeWidgetItem;
class FilesTabWidget;

class SearchPanel final : public QWidget {
    Q_OBJECT

public:
    explicit SearchPanel(const QString& projectPath, FilesTabWidget* filesTabWidget, QWidget* parent = nullptr);

    void open(SearchScope scope, bool replaceMode, const QString& initialQuery = {});
    void nextResult();
    void previousResult();

signals:
    void closeRequested();
    void statusMessage(const QString& message);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private slots:
    void scheduleSearch();
    void performSearch();
    void activateCurrentResult();
    void replaceCurrentResult();
    void replaceAllResults();

private:
    SearchOptions currentOptions() const;
    void buildUi();
    void setReplaceMode(bool enabled);
    void setSearching(bool searching);
    void showReport(const SearchReport& report);
    void showEmptyState(const QString& title, const QString& detail = {});
    void selectAdjacentResult(bool forward);
    int selectedMatchIndex() const;
    bool writeDocument(const QString& filePath, const QByteArray& contents, QString* error);
    QByteArray readDocument(const QString& filePath, bool* ok) const;
    QString displayPath(const QString& filePath) const;

    QString m_projectPath;
    FilesTabWidget* m_filesTabWidget = nullptr;
    QLineEdit* m_searchEdit = nullptr;
    QLineEdit* m_replaceEdit = nullptr;
    QComboBox* m_scopeCombo = nullptr;
    QToolButton* m_replaceToggle = nullptr;
    QToolButton* m_matchCaseButton = nullptr;
    QToolButton* m_wholeWordButton = nullptr;
    QToolButton* m_regexButton = nullptr;
    QLineEdit* m_includeEdit = nullptr;
    QLineEdit* m_excludeEdit = nullptr;
    QWidget* m_filterRow = nullptr;
    QWidget* m_replaceRow = nullptr;
    QPushButton* m_replaceButton = nullptr;
    QPushButton* m_replaceAllButton = nullptr;
    QPushButton* m_previousButton = nullptr;
    QPushButton* m_nextButton = nullptr;
    QLabel* m_statusLabel = nullptr;
    QLabel* m_emptyTitle = nullptr;
    QLabel* m_emptyDetail = nullptr;
    QTreeWidget* m_resultsTree = nullptr;
    QStackedWidget* m_resultsStack = nullptr;
    QTimer* m_searchTimer = nullptr;
    SearchReport m_report;
    int m_searchGeneration = 0;
    bool m_replaceMode = false;
};
