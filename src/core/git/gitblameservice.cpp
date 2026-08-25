#include "gitblameservice.h"

#include "core/settings/appsettings.h"
#include "internal/gitblameengine.h"
#include "internal/gitrepository.h"

#include <QDir>
#include <QFileInfo>
#include <QFutureWatcher>
#include <QtConcurrentRun>

namespace {
    struct BlameRequestResult {
        QVector<BlameLineInfo> lines;
        QString error;
    };
}// namespace

GitBlameService* GitBlameService::instance() {
    static GitBlameService service;
    return &service;
}

QString GitBlameService::enabledSettingKey() {
    // Keep the existing key so upgrades retain the user's preference and old
    // exported settings remain importable. Ownership now belongs to Git core.
    return QStringLiteral("modules/codeEditor/gitBlameEnabled");
}

GitBlameService::GitBlameService(QObject* parent)
    : QObject(parent) {
    connect(SettingsNotifier::instance(), &SettingsNotifier::settingsChanged,
            this, [this](const QString& key) {
                if (key == enabledSettingKey())
                    emit enabledChanged(isEnabled());
            });
}

bool GitBlameService::isEnabled() const {
    return AppSettings::value(enabledSettingKey(), false).toBool();
}

void GitBlameService::setEnabled(bool enabled) {
    if (isEnabled() == enabled)
        return;

    AppSettings::setValue(enabledSettingKey(), enabled);
    if (!enabled)
        m_requestVersions.clear();
    emit SettingsNotifier::instance() -> settingsChanged(enabledSettingKey());
}

void GitBlameService::requestBlame(const QString& filePath) {
    if (filePath.isEmpty()) {
        emit blameReady({}, {});
        return;
    }

    const QString absolutePath = QFileInfo(filePath).absoluteFilePath();
    const quint64 requestVersion = ++m_nextRequestVersion;
    m_requestVersions.insert(absolutePath, requestVersion);

    auto* watcher = new QFutureWatcher<BlameRequestResult>(this);
    connect(watcher, &QFutureWatcher<BlameRequestResult>::finished,
            this, [this, watcher, absolutePath, requestVersion]() {
                const BlameRequestResult result = watcher->result();
                watcher->deleteLater();

                if (m_requestVersions.value(absolutePath) != requestVersion)
                    return;

                m_requestVersions.remove(absolutePath);
                if (result.error.isEmpty())
                    emit blameReady(absolutePath, result.lines);
                else
                    emit blameFailed(absolutePath, result.error);
            });

    watcher->setFuture(QtConcurrent::run([absolutePath]() {
        BlameRequestResult result;
        const QString repoRoot = GitInternal::Repository::discoverRoot(
            QFileInfo(absolutePath).absolutePath());
        if (repoRoot.isEmpty())
            return result;

        GitInternal::Repository repository;
        if (!repository.open(repoRoot)) {
            result.error = repository.lastError();
            return result;
        }

        const QString relativePath = QDir(repoRoot).relativeFilePath(absolutePath);
        result.lines = GitInternal::BlameEngine::blameFile(repository, relativePath);
        if (result.lines.isEmpty() && !repository.lastError().isEmpty())
            result.error = repository.lastError();
        return result;
    }));
}
