// SPDX-FileCopyrightText: 2023 Claudio Cambra <claudio.cambra@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "abstractcryptographybackend.h"

namespace KIdentityManagement
{
namespace Quick
{

AbstractCryptographyBackend::AbstractCryptographyBackend(QObject *parent)
    : QObject(parent)
{
}

AbstractKeyListModel *AbstractCryptographyBackend::openPgpKeyListModel() const
{
    return nullptr;
}

AbstractKeyListModel *AbstractCryptographyBackend::smimeKeyListModel() const
{
    return nullptr;
}

}
}
