// SPDX-FileCopyrightText: 2023 Claudio Cambra <claudio.cambra@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "cryptographyeditorbackend.h"

#include "cryptographybackendinterface.h"
#include "identity.h"

namespace KIdentityManagementQuick
{

CryptographyEditorBackend::CryptographyEditorBackend(QObject *parent, const CryptographyBackendInterfacePtr &cryptoBackend)
    : QObject{parent}
    , mCryptoBackend(cryptoBackend)
{
    connect(this, &CryptographyEditorBackend::cryptographyBackendChanged, this, &CryptographyEditorBackend::openPgpKeyListModelChanged);
    connect(this, &CryptographyEditorBackend::cryptographyBackendChanged, this, &CryptographyEditorBackend::smimeKeyListModelChanged);
}

CryptographyBackendInterfacePtr CryptographyEditorBackend::cryptographyBackend() const
{
    return mCryptoBackend;
}

void CryptographyEditorBackend::setCryptographyBackend(const CryptographyBackendInterfacePtr &cryptoBackend)
{
    if (cryptoBackend.get() == mCryptoBackend.get()) {
        return;
    }

    mCryptoBackend = cryptoBackend;
    Q_EMIT cryptographyBackendChanged();
}

QAbstractItemModel *CryptographyEditorBackend::openPgpKeyListModel() const
{
    if (!mCryptoBackend) {
        return nullptr;
    }
    return mCryptoBackend->openPgpKeyListModel();
}

QAbstractItemModel *CryptographyEditorBackend::smimeKeyListModel() const
{
    if (!mCryptoBackend) {
        return nullptr;
    }
    return mCryptoBackend->smimeKeyListModel();
}

KIdentityManagementCore::Identity CryptographyEditorBackend::identity() const
{
    return mCryptoBackend->identity();
}

void CryptographyEditorBackend::setIdentity(const KIdentityManagementCore::Identity &identity)
{
    mCryptoBackend->setIdentity(identity);
    Q_EMIT identityChanged();
}

QModelIndex
CryptographyEditorBackend::indexForIdentity(QAbstractItemModel *model, const KIdentityManagementCore::Identity &identity, const KeyUseTypes::KeyUse keyUse)
{
    Q_ASSERT(model);
    const auto klmInterface = dynamic_cast<const KeyListModelInterface *>(model);
    Q_ASSERT(klmInterface);
    return klmInterface->indexForIdentity(identity, keyUse);
}

QString CryptographyEditorBackend::stringFromKeyByteArray(const QByteArray &key)
{
    return QString::fromUtf8(key);
}
}

#include "moc_cryptographyeditorbackend.cpp"
