#ifndef GITBLAMEWORKER_H
#define GITBLAMEWORKER_H

#include <QObject>
#include <QString>
#include <QVector>
#include "gitmanager.h"

/**
 * @brief Background worker for git blame operations
 */
class GitBlameWorker : public QObject
{
    Q_OBJECT

public:
    explicit GitBlameWorker(QObject *parent = nullptr);

public slots:
    /**
     * @brief Run blame for the given file
     * @param repoRoot Root of the git repository
     * @param filePath Full path to the file
     */
    void runBlame(const QString &repoRoot, const QString &filePath);

signals:
    /**
     * @brief Emitted when blame calculation is finished
     * @param result Vector of blame info per line
     */
    void blameFinished(const QVector<BlameLineInfo> &result);

    /**
     * @brief Emitted when an error occurs during blame
     * @param error Error message
     */
    void errorOccurred(const QString &error);
};

#endif // GITBLAMEWORKER_H
