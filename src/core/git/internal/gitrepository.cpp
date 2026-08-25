#include "gitrepository.h"

#include <QCoreApplication>
#include <git2.h>

namespace {
    QString gitTr(const char* text) {
        return QCoreApplication::translate("GitManager", text);
    }
}// namespace

namespace GitInternal {

    Repository::Repository() {
        git_libgit2_init();
    }

    Repository::~Repository() {
        close();
        git_libgit2_shutdown();
    }

    bool Repository::open(const QString& path) {
        close();

        git_repository* repository = nullptr;
        const int error = git_repository_open(&repository, path.toUtf8().constData());
        if (error != 0) {
            const git_error* gitError = git_error_last();
            setError(gitError ? QString::fromUtf8(gitError->message)
                              : gitTr("Failed to open repository"));
            return false;
        }

        adopt(repository, path);
        clearError();
        return true;
    }

    void Repository::close() {
        if (m_repository)
            git_repository_free(m_repository);
        m_repository = nullptr;
        m_path.clear();
    }

    void Repository::adopt(git_repository* repository, const QString& path) {
        if (m_repository != repository)
            close();
        m_repository = repository;
        m_path = path;
    }

    bool Repository::isOpen() const {
        return m_repository != nullptr;
    }

    git_repository* Repository::handle() const {
        return m_repository;
    }

    QString Repository::path() const {
        return m_path;
    }

    QString Repository::lastError() const {
        return m_lastError;
    }

    void Repository::setError(const QString& error) const {
        m_lastError = error;
    }

    void Repository::clearError() const {
        m_lastError.clear();
    }

    QString Repository::userName() const {
        if (!m_repository)
            return QStringLiteral("User");

        git_config* config = nullptr;
        if (git_repository_config(&config, m_repository) != 0)
            return QStringLiteral("User");

        const char* value = nullptr;
        const int error = git_config_get_string(&value, config, "user.name");
        const QString name = (error == 0 && value)
                                 ? QString::fromUtf8(value)
                                 : QStringLiteral("User");
        git_config_free(config);
        return name;
    }

    QString Repository::userEmail() const {
        if (!m_repository)
            return QStringLiteral("user@example.com");

        git_config* config = nullptr;
        if (git_repository_config(&config, m_repository) != 0)
            return QStringLiteral("user@example.com");

        const char* value = nullptr;
        const int error = git_config_get_string(&value, config, "user.email");
        const QString email = (error == 0 && value)
                                  ? QString::fromUtf8(value)
                                  : QStringLiteral("user@example.com");
        git_config_free(config);
        return email;
    }

    git_signature* Repository::createSignature() const {
        git_signature* signature = nullptr;
        const QByteArray name = userName().toUtf8();
        const QByteArray email = userEmail().toUtf8();
        if (git_signature_now(&signature, name.constData(), email.constData()) != 0) {
            setError(gitTr("Failed to create signature"));
            return nullptr;
        }
        return signature;
    }

}// namespace GitInternal
