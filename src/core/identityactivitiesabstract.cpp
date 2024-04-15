// SPDX-FileCopyrightText: 2024 Laurent Montel <montel@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "identityactivitiesabstract.h"

using namespace KIdentityManagementCore;
IdentityActivitiesAbstract::IdentityActivitiesAbstract(QObject *parent)
    : QObject{parent}
{
}

IdentityActivitiesAbstract::~IdentityActivitiesAbstract() = default;

bool IdentityActivitiesAbstract::hasActivitySupport() const
{
    return false;
}

#include "moc_identityactivitiesabstract.cpp"
