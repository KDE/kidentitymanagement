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

bool IdentityEditorBackend::signatureEnabled() const
{
    return mIdentity.signature().isEnabledSignature();
}

void IdentityEditorBackend::setSignatureEnabled(bool enabled)
{
    if (signatureEnabled() == enabled) {
        return;
    }
    auto &signature = mIdentity.signature();
    if (enabled && signature.type() == KIdentityManagementCore::Signature::Disabled) {
        signature.setType(KIdentityManagementCore::Signature::Inlined);
    }
    signature.setEnabledSignature(enabled);
    Q_EMIT identityChanged();
}

QString IdentityEditorBackend::signatureText() const
{
    const auto &signature = mIdentity.signature();
    return signature.type() == KIdentityManagementCore::Signature::Inlined ? signature.toPlainText() : QString();
}

void IdentityEditorBackend::setSignatureText(const QString &text)
{
    if (signatureText() == text) {
        return;
    }
    auto &signature = mIdentity.signature();
    signature.setText(text);
    signature.setInlinedHtml(false);
    Q_EMIT identityChanged();
}

#include "moc_identityeditorbackend.cpp"
