// SPDX-FileCopyrightText: 2023 Claudio Cambra <claudio.cambra@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "identityutils.h"

#include <QTimer>

namespace KIdentityManagementQuick
{
bool IdentityUtils::removeIdentity(const QString &identityName)
{
    if (!mIdentityManager) {
        return false;
    }

    const auto result = mIdentityManager->removeIdentity(identityName);
    // Need to run async or will crash the UI if this is called from QML
    QTimer::singleShot(0, mIdentityManager, &KIdentityManagementCore::IdentityManager::commit);
    return result;
}

}

#include "moc_identityutils.cpp"
