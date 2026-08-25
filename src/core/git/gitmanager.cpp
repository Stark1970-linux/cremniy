#include "gitmanager.h"
#include "internal/gitblameengine.h"
#include "internal/gitbranchservice.h"
#include "internal/gitcommitservice.h"
#include "internal/gitindexservice.h"
#include "internal/gitmergeservice.h"
#include "internal/gitremoteservice.h"
#include "internal/gitrepository.h"
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <git2.h>

GitManager::GitManager(QObject* parent)
    : QObject(parent), m_repository(std::make_unique<GitInternal::Repository>()) {
}

GitManager::~GitManager() = default;

bool GitManager::open(const QString& repoPath) {
    return m_repository->open(repoPath);
}

void GitManager::close() {
    m_repository->close();
}

bool GitManager::isOpen() const {
    return m_repository->isOpen();
}

QString GitManager::lastError() const {
    return m_repository->lastError();
}

QString GitManager::repoPath() const {
    return m_repository->path();
}

void GitManager::setError(const QString& error) const {
    m_repository->setError(error);
}

// Ветки

QStringList GitManager::branches() const {
    return GitInternal::BranchService::branches(*m_repository);
}

QString GitManager::currentBranch() const {
    return GitInternal::BranchService::currentBranch(*m_repository);
}

bool GitManager::checkoutBranch(const QString& branchName) {
    const bool success = GitInternal::BranchService::checkout(*m_repository, branchName);
    if (success)
        emit repositoryChanged();
    return success;
}

bool GitManager::createBranch(const QString& branchName) {
    const bool success = GitInternal::BranchService::create(*m_repository, branchName);
    if (success)
        emit repositoryChanged();
    return success;
}

bool GitManager::deleteBranch(const QString& branchName) {
    const bool success = GitInternal::BranchService::remove(*m_repository, branchName);
    if (success)
        emit repositoryChanged();
    return success;
}

bool GitManager::renameBranch(const QString& oldName, const QString& newName) {
    const bool success = GitInternal::BranchService::rename(
        *m_repository, oldName, newName);
    if (success)
        emit repositoryChanged();
    return success;
}

// Коммиты

bool GitManager::createCommit(const QString& message) {
    const bool success = GitInternal::CommitService::create(*m_repository, message);
    if (success)
        emit repositoryChanged();
    return success;
}

QStringList GitManager::commitHistory(int count) const {
    return GitInternal::CommitService::history(*m_repository, count);
}

QString GitManager::commitMessage(const QString& oid) const {
    return GitInternal::CommitService::message(*m_repository, oid);
}

QString GitManager::commitAuthor(const QString& oid) const {
    return GitInternal::CommitService::author(*m_repository, oid);
}

bool GitManager::checkoutCommit(const QString& oid) {
    const bool success = GitInternal::CommitService::checkout(*m_repository, oid);
    if (success)
        emit repositoryChanged();
    return success;
}

bool GitManager::resetHard(const QString& oid) {
    const bool success = GitInternal::CommitService::resetHard(*m_repository, oid);
    if (success)
        emit repositoryChanged();
    return success;
}

bool GitManager::resetMixed(const QString& oid) {
    const bool success = GitInternal::CommitService::resetMixed(*m_repository, oid);
    if (success)
        emit repositoryChanged();
    return success;
}

bool GitManager::revertCommit(const QString& oid) {
    const bool success = GitInternal::CommitService::revert(*m_repository, oid);
    if (success)
        emit repositoryChanged();
    return success;
}

bool GitManager::amendCommit(const QString& message) {
    const bool success = GitInternal::CommitService::amend(*m_repository, message);
    if (success)
        emit repositoryChanged();
    return success;
}

// Синхронизация

bool GitManager::push(const QString& remote, const QString& branch) {
    const QString branchRef = branch.isEmpty() ? currentBranch() : branch;
    if (branchRef.isEmpty()) {
        setError(tr("Push branch not specified"));
        return false;
    }
    return GitInternal::RemoteService::push(*m_repository, remote, branchRef);
}

bool GitManager::pull(const QString& remote, const QString& branch) {
    if (!m_repository->isOpen()) {
        setError(tr("Repository not open"));
        return false;
    }

    // сначала получаем изменения
    if (!fetch(remote))
        return false;

    // потом сливаем
    QString branchRef = branch.isEmpty() ? currentBranch() : branch;
    if (branchRef.isEmpty()) {
        setError(tr("Branch not specified"));
        return false;
    }

    QString remoteRef = "refs/remotes/" + remote + "/" + branchRef;
    return merge(remoteRef);
}

bool GitManager::fetch(const QString& remote) {
    return GitInternal::RemoteService::fetch(*m_repository, remote);
}

// Слияние

bool GitManager::merge(const QString& branchName) {
    const bool success = GitInternal::MergeService::merge(*m_repository, branchName);
    if (success)
        emit repositoryChanged();
    return success;
}

bool GitManager::hasConflicts() const {
    return GitInternal::IndexService::hasConflicts(*m_repository);
}

QStringList GitManager::conflictFiles() const {
    return GitInternal::IndexService::conflictFiles(*m_repository);
}

// Индексация

bool GitManager::stageFile(const QString& filePath) {
    const bool success = GitInternal::IndexService::stage(*m_repository, filePath);
    if (success)
        emit repositoryChanged();
    return success;
}

bool GitManager::unstageFile(const QString& filePath) {
    const bool success = GitInternal::IndexService::unstage(*m_repository, filePath);
    if (success)
        emit repositoryChanged();
    return success;
}

QString GitManager::fileDiff(const QString& filePath) const {
    return GitInternal::IndexService::fileDiff(*m_repository, filePath);
}

QString GitManager::stagedDiff() const {
    return GitInternal::IndexService::stagedDiff(*m_repository);
}

// Репозиторий

bool GitManager::clone(const QString& url, const QString& path) {
    const bool success = m_repository->clone(url, path);
    if (success)
        emit repositoryChanged();
    return success;
}

bool GitManager::init(const QString& path) {
    const bool success = m_repository->init(path);
    if (success)
        emit repositoryChanged();
    return success;
}

QString GitManager::findGitRepositoryRoot(const QString& path) {
    return GitInternal::Repository::discoverRoot(path);
}

QVector<BlameLineInfo> GitManager::blameFile(const QString& relativeFilePath) const {
    return GitInternal::BlameEngine::blameFile(*m_repository, relativeFilePath);
}

// Дополнительно

QString GitManager::status() const {
    return GitInternal::IndexService::status(*m_repository, currentBranch());
}

bool GitManager::stashSave(const QString& message) {
    if (!m_repository->isOpen()) {
        setError(tr("Repository not open"));
        return false;
    }

    git_signature* sig = createSignature();
    if (!sig)
        return false;

    int error = git_stash_save(nullptr, m_repository->handle(), sig, message.isEmpty() ? nullptr : message.toUtf8().constData(), GIT_STASH_DEFAULT);
    git_signature_free(sig);

    if (error != 0) {
        const git_error* e = git_error_last();
        setError(e ? QString::fromUtf8(e->message) : tr("Stash save error"));
        return false;
    }

    emit repositoryChanged();
    return true;
}

bool GitManager::stashApply(int index) {
    if (!m_repository->isOpen()) {
        setError(tr("Repository not open"));
        return false;
    }

    int error = git_stash_apply(m_repository->handle(), index, nullptr);
    if (error != 0) {
        const git_error* e = git_error_last();
        setError(e ? QString::fromUtf8(e->message) : tr("Stash apply error"));
        return false;
    }

    emit repositoryChanged();
    return true;
}

bool GitManager::stashDrop(int index) {
    if (!m_repository->isOpen()) {
        setError(tr("Repository not open"));
        return false;
    }

    int error = git_stash_drop(m_repository->handle(), index);
    if (error != 0) {
        const git_error* e = git_error_last();
        setError(e ? QString::fromUtf8(e->message) : tr("Stash delete error"));
        return false;
    }

    return true;
}

QStringList GitManager::stashList() const {
    QStringList result;
    if (!m_repository->isOpen())
        return result;

    git_revwalk* walker = nullptr;
    if (git_revwalk_new(&walker, m_repository->handle()) != 0)
        return result;

    git_revwalk_sorting(walker, GIT_SORT_TIME);
    git_revwalk_push_ref(walker, "refs/stash");

    git_oid oid;
    while (git_revwalk_next(&oid, walker) == 0) {
        git_commit* commit = nullptr;
        if (git_commit_lookup(&commit, m_repository->handle(), &oid) == 0) {
            QString msg = QString::fromUtf8(git_commit_message(commit));
            result.append(msg.trimmed());
            git_commit_free(commit);
        }
    }

    git_revwalk_free(walker);
    return result;
}

QString GitManager::logGraph(int count) const {
    if (!m_repository->isOpen())
        return {};

    git_revwalk* walker = nullptr;
    int error = git_revwalk_new(&walker, m_repository->handle());
    if (error != 0)
        return {};

    git_revwalk_sorting(walker, GIT_SORT_TIME | GIT_SORT_TOPOLOGICAL);
    git_revwalk_push_head(walker);

    // получаем все ветки для пометок
    QStringList branchList = branches();
    QString currentBranch = this->currentBranch();

    QString result;
    git_oid oid;
    int i = 0;

    while (git_revwalk_next(&oid, walker) == 0 && i < count) {
        git_commit* commit = nullptr;
        if (git_commit_lookup(&commit, m_repository->handle(), &oid) != 0)
            continue;

        const git_signature* sig = git_commit_author(commit);
        QString msg = QString::fromUtf8(git_commit_message(commit)).split('\n').first();
        QString oidStr = QString::fromUtf8(git_oid_tostr_s(&oid));
        QString author = QString::fromUtf8(sig->name);
        QString dateStr = QDateTime::fromSecsSinceEpoch(sig->when.time).toString("yyyy-MM-dd HH:mm");

        // проверяем, указывает ли ветка на этот коммит
        QStringList refs;
        for (const QString& branch: branchList) {
            git_oid branch_oid;
            QString refName = "refs/heads/" + branch;
            if (git_reference_name_to_id(&branch_oid, m_repository->handle(), refName.toUtf8().constData()) == 0) {
                if (git_oid_equal(&oid, &branch_oid)) {
                    if (branch == currentBranch) {
                        refs.prepend("* " + branch);
                    }
                    else {
                        refs.append(branch);
                    }
                }
            }
        }

        QString refStr;
        if (!refs.isEmpty()) {
            refStr = " (" + refs.join(", ") + ")";
        }

        result += QString("* %1 %2%3\n  | %4 <%5>\n  | %6\n")
                      .arg(oidStr.left(7))
                      .arg(dateStr)
                      .arg(refStr)
                      .arg(author)
                      .arg(QString::fromUtf8(sig->email))
                      .arg(msg);

        git_commit_free(commit);
        i++;
    }

    git_revwalk_free(walker);
    return result;
}

git_signature* GitManager::createSignature() const {
    return m_repository->createSignature();
}
