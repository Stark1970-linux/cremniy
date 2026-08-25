#ifndef BLAMELINEINFO_H
#define BLAMELINEINFO_H

#include <QString>
#include <QDateTime>

/**
 * @brief Information about a single line blame
 */
struct BlameLineInfo {
    QString authorName;
    QString authorEmail;
    QDateTime commitDate;
    QString shortOid;
    QString fullOid;
    QString commitSummary;
    bool isUncommitted = false;
};

#endif // BLAMELINEINFO_H
