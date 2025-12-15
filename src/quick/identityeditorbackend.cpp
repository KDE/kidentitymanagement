// SPDX-FileCopyrightText: 2023 Claudio Cambra <claudio.cambra@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "identityeditorbackend.h"

using namespace KIdentityManagementQuick;
IdentityEditorBackend::Mode IdentityEditorBackend::mode() const
{
    return mMode;
}

void IdentityEditorBackend::setMode(Mode mode)
{
    if (mMode == mode) {
        return;
    }

    mMode = mode;
    Q_EMIT modeChanged();
}

KIdentityManagementCore::Identity IdentityEditorBackend::identity() const
{
    return mIdentity;
}

void IdentityEditorBackend::setIdentity(const KIdentityManagementCore::Identity &identity)
{
    if (mIdentity == identity) {
        return;
    }

    mIdentity = identity;
    Q_EMIT identityChanged();
}

uint IdentityEditorBackend::identityUoid() const
{
    return mIdentity.uoid();
}

void IdentityEditorBackend::setIdentityUoid(uint identityUoid)
{
    if (mIdentity.uoid() == identityUoid) {
        return;
    }

    const auto &identity = mIdentityManager->modifyIdentityForUoid(identityUoid);
    setIdentity(identity);
}

void IdentityEditorBackend::newIdentity(const KIdentityManagementCore::Identity &newIdentity)
{
    auto identity = mIdentityManager->newFromExisting(newIdentity, newIdentity.identityName());
    mIdentityManager->saveIdentity(identity);
}

void IdentityEditorBackend::saveIdentity(const KIdentityManagementCore::Identity &modifiedIdentity)
{
    mIdentityManager->saveIdentity(modifiedIdentity);
}

void IdentityEditorBackend::addEmailAlias(const QString &alias)
{
    auto aliases = mIdentity.emailAliases();
    aliases.append(alias);
    mIdentity.setEmailAliases(aliases);
}

void IdentityEditorBackend::modifyEmailAlias(const QString &originalAlias, const QString &modifiedAlias)
{
    auto aliases = mIdentity.emailAliases();
    std::replace(aliases.begin(), aliases.end(), originalAlias, modifiedAlias);
    mIdentity.setEmailAliases(aliases);
}

void IdentityEditorBackend::removeEmailAlias(const QString &alias)
{
    auto aliases = mIdentity.emailAliases();
    aliases.removeAll(alias);
    mIdentity.setEmailAliases(aliases);
}

#include "moc_identityeditorbackend.cpp"
