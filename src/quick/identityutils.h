// SPDX-FileCopyrightText: 2023 Claudio Cambra <claudio.cambra@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#pragma once

#include <QObject>

#include "identitymanager.h"

namespace KIdentityManagementQuick
{
class IdentityUtils : public QObject
{
    Q_OBJECT

public:
    explicit IdentityUtils() = default;

    Q_INVOKABLE bool removeIdentity(const QString &identityName);

private:
    KIdentityManagementCore::IdentityManager *const mIdentityManager = KIdentityManagementCore::IdentityManager::self();
};
}
