// SPDX-FileCopyrightText: 2023 Claudio Cambra <claudio.cambra@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "cryptographyeditorbackend.h"

#include "cryptographybackendinterface.h"

namespace KIdentityManagement
{
namespace Quick
{

CryptographyEditorBackend::CryptographyEditorBackend(QObject *parent, const CryptographyBackendInterfacePtr &cryptoBackend)
    : QObject{parent}
    , m_cryptoBackend(cryptoBackend)
{
    connect(this, &CryptographyEditorBackend::cryptographyBackendChanged, this, &CryptographyEditorBackend::openPgpKeyListModelChanged);
    connect(this, &CryptographyEditorBackend::cryptographyBackendChanged, this, &CryptographyEditorBackend::smimeKeyListModelChanged);
}

CryptographyBackendInterfacePtr CryptographyEditorBackend::cryptographyBackend() const
{
    return m_cryptoBackend;
}

void CryptographyEditorBackend::setCryptographyBackend(const CryptographyBackendInterfacePtr &cryptoBackend)
{
    if (cryptoBackend.get() == m_cryptoBackend.get()) {
        return;
    }

    m_cryptoBackend = cryptoBackend;
    Q_EMIT cryptographyBackendChanged();
}

QAbstractItemModel *CryptographyEditorBackend::openPgpKeyListModel() const
{
    if (!m_cryptoBackend) {
        return nullptr;
    }
    return m_cryptoBackend->openPgpKeyListModel();
}

QAbstractItemModel *CryptographyEditorBackend::smimeKeyListModel() const
{
    if (!m_cryptoBackend) {
        return nullptr;
    }
    return m_cryptoBackend->smimeKeyListModel();
}

Identity CryptographyEditorBackend::identity() const
{
    return m_cryptoBackend->identity();
}

void CryptographyEditorBackend::setIdentity(const Identity &identity)
{
    m_cryptoBackend->setIdentity(identity);
    Q_EMIT identityChanged();
}
}
}
