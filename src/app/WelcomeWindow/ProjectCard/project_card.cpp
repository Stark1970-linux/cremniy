/*
 * This file is part of the Cremniy IDE source code.
 *
 * Copyright (c) 2026 Cremniy IDE
 * SPDX-License-Identifier: GPL-3.0 license
 *
 * Repository:
 * https://github.com/munirov/cremniy
 *
 * Modified by Ilya (https://github.com/kykyrudza) on 2026-05-12
 */


#include "project_card.h"

#include <QApplication>
#include <QColor>
#include <QFileInfo>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include "core/theme/thememanager.h"

ProjectCard::ProjectCard(const utils::RecentProject& project, QWidget* parent)
    : QWidget(parent)
    , m_path(project.path)
{
    setFixedHeight(72);
    setObjectName("ProjectCard");

    auto* root = new QHBoxLayout(this);
    root->setContentsMargins(14, 8, 14, 8);
    root->setSpacing(12);

    const QString language = project.language.isEmpty() ? "?" : project.language;
    m_language = language;

    auto* badge = new QLabel(shortLang(language));
    m_badge = badge;
    badge->setObjectName("ProjectCardBadge");
    badge->setFixedSize(42, 42);
    badge->setAlignment(Qt::AlignCenter);
    badge->setProperty("language", language);
    root->addWidget(badge);
    applyBadgeColor();

    connect(&ThemeManager::instance(), &ThemeManager::currentThemeChanged,
            this, [this](const QString&) { applyBadgeColor(); });
    connect(&ThemeManager::instance(), &ThemeManager::themePreviewChanged,
            this, [this]() { applyBadgeColor(); });

    auto* info = new QVBoxLayout();
    info->setSpacing(3);
    info->setContentsMargins(0, 0, 0, 0);

    const QString displayName = project.name.isEmpty()
        ? QFileInfo(project.path).fileName()
        : project.name;

    auto* nameLabel = new QLabel(displayName);
    nameLabel->setObjectName("ProjectCardName");

    auto* pathLabel = new QLabel(project.path);
    pathLabel->setObjectName("ProjectCardPath");
    pathLabel->setToolTip(project.path);

    QString lastOpened = project.lastOpenedAt;
    if (!lastOpened.isEmpty()) {
        lastOpened = lastOpened.replace("T", " ").left(16);
    }

    auto* lastLabel = new QLabel(lastOpened);
    lastLabel->setObjectName("ProjectCardLastOpened");

    info->addWidget(nameLabel);
    info->addWidget(pathLabel);
    info->addWidget(lastLabel);
    root->addLayout(info, 1);

    auto* openBtn = new QPushButton(tr("Open"));
    openBtn->setObjectName("ProjectCardOpenBtn");
    openBtn->setCursor(Qt::PointingHandCursor);
    root->addWidget(openBtn, 0, Qt::AlignVCenter);

    auto* removeBtn = new QPushButton(tr("Remove"));
    removeBtn->setObjectName("ProjectCardRemoveBtn");
    removeBtn->setCursor(Qt::PointingHandCursor);
    root->addWidget(removeBtn, 0, Qt::AlignVCenter);

    connect(openBtn, &QPushButton::clicked, this, [this]() {
        emit openRequested(m_path);
    });

    connect(removeBtn, &QPushButton::clicked, this, [this]() {
        emit removeRequested(m_path);
    });
}

void ProjectCard::applyBadgeColor()
{
    if (!m_badge)
        return;

    const QByteArray propertyName = (QStringLiteral("cremniyProjectIconColor_") + m_language).toLatin1();
    QColor color = qApp->property(propertyName.constData()).value<QColor>();
    if (!color.isValid())
        color = qApp->property("cremniyProjectIconColor_?").value<QColor>();
    if (!color.isValid())
        color = QColor("#3A3A3A");

    m_badge->setStyleSheet(QStringLiteral(
        "QLabel#ProjectCardBadge { background-color: %1; }"
    ).arg(color.name(QColor::HexRgb)));
}

QString ProjectCard::shortLang(const QString& lang)
{
    if (lang == "C++") return "CPP";
    if (lang == "C") return "C";
    if (lang == "ASM") return "ASM";
    if (lang == "C + ASM") return "C+ASM";
    if (lang == "Rust") return "RS";
    if (lang == "Custom") return "USR";
    if (lang == "?") return "?";
    return lang;
}
