#pragma once

#include "blamelineinfo.h"

#include <QHash>
#include <QObject>
#include <QString>
#include <QVector>

/**
 * @brief Application-facing interface for asynchronous Git blame requests.
 *
 * The service owns background execution, shared enablement state and
 * invalidation events. Consumers only request blame for an absolute file path
 * and receive a correlated result; they never manage repositories or threads.
 */
class GitBlameService final : public QObject {
    Q_OBJECT

public:
    static GitBlameService* instance();

    static QString enabledSettingKey();

    bool isEnabled() const;
    void setEnabled(bool enabled);

    void requestBlame(const QString& filePath);

signals:
    void enabledChanged(bool enabled);
    void blameReady(const QString& filePath, const QVector<BlameLineInfo>& result);
    void blameFailed(const QString& filePath, const QString& error);
    void repositoryChanged();

private:
    explicit GitBlameService(QObject* parent = nullptr);

    QHash<QString, quint64> m_requestVersions;
    quint64 m_nextRequestVersion = 0;
};
