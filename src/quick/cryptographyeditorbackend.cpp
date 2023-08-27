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
}
}
