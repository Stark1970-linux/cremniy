#include "gitmanager.h"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QtTest>

class GitCoreTest : public QObject {
    Q_OBJECT

private slots:
    void discoversRepositoryFromNestedDirectory();
    void discoversWorktreeMarkerFile();
    void returnsEmptyOutsideRepository();
    void ownsRepositoryLifecycle();
};

void GitCoreTest::discoversRepositoryFromNestedDirectory() {
    QTemporaryDir temporaryDirectory;
    QVERIFY(temporaryDirectory.isValid());

    QDir root(temporaryDirectory.path());
    QVERIFY(root.mkpath(QStringLiteral("repo/.git")));
    QVERIFY(root.mkpath(QStringLiteral("repo/src/nested")));

    const QString repoPath = root.absoluteFilePath(QStringLiteral("repo"));
    const QString nestedPath = root.absoluteFilePath(QStringLiteral("repo/src/nested"));
    QCOMPARE(GitManager::findGitRepositoryRoot(nestedPath), QDir(repoPath).absolutePath());
}

void GitCoreTest::discoversWorktreeMarkerFile() {
    QTemporaryDir temporaryDirectory;
    QVERIFY(temporaryDirectory.isValid());

    QDir root(temporaryDirectory.path());
    QVERIFY(root.mkpath(QStringLiteral("worktree/src")));

    QFile marker(root.absoluteFilePath(QStringLiteral("worktree/.git")));
    QVERIFY(marker.open(QIODevice::WriteOnly));
    QVERIFY(marker.write("gitdir: ../repository/.git/worktrees/worktree\n") > 0);
    marker.close();

    const QString repoPath = root.absoluteFilePath(QStringLiteral("worktree"));
    const QString sourcePath = root.absoluteFilePath(QStringLiteral("worktree/src"));
    QCOMPARE(GitManager::findGitRepositoryRoot(sourcePath), QDir(repoPath).absolutePath());
}

void GitCoreTest::returnsEmptyOutsideRepository() {
    QTemporaryDir temporaryDirectory;
    QVERIFY(temporaryDirectory.isValid());
    QVERIFY(GitManager::findGitRepositoryRoot(temporaryDirectory.path()).isEmpty());
}

void GitCoreTest::ownsRepositoryLifecycle() {
    QTemporaryDir temporaryDirectory(QDir::current().absoluteFilePath(QStringLiteral("gitcoretest-XXXXXX")));
    QVERIFY(temporaryDirectory.isValid());

    const QString repositoryPath = QDir(temporaryDirectory.path()).absoluteFilePath(QStringLiteral("repository"));
    QVERIFY(QDir().mkpath(repositoryPath));
    GitManager git;
    QVERIFY2(git.init(repositoryPath), qPrintable(git.lastError()));
    QVERIFY(git.isOpen());
    QCOMPARE(QDir(git.repoPath()).absolutePath(), QDir(repositoryPath).absolutePath());

    git.close();
    QVERIFY(!git.isOpen());
    QVERIFY(git.repoPath().isEmpty());
}

QTEST_GUILESS_MAIN(GitCoreTest)
#include "gitcoretest.moc"
