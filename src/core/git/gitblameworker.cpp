#include "gitblameworker.h"
#include <QFileInfo>
#include <QDir>

GitBlameWorker::GitBlameWorker(QObject *parent)
    : QObject(parent)
{
}

void GitBlameWorker::runBlame(const QString &repoRoot, const QString &filePath)
{
    GitManager git;
    if (!git.open(repoRoot)) {
        emit errorOccurred(git.lastError());
        return;
    }

    QDir rootDir(repoRoot);
    QString relativePath = rootDir.relativeFilePath(filePath);

    QVector<BlameLineInfo> result = git.blameFile(relativePath);
    if (result.isEmpty() && !git.lastError().isEmpty()) {
        emit errorOccurred(git.lastError());
    } else {
        emit blameFinished(result);
    }
}
