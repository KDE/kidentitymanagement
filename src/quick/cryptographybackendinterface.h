// SPDX-FileCopyrightText: 2023 Claudio Cambra <claudio.cambra@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QObject>

class QAbstractItemModel;

namespace KIdentityManagement
{
namespace Quick
{

/**
 * Defines the interface expected by the Cryptography Editor Backend.
 * This class is instantiated by the CryptographyBackend, which exposes
 * what is needed to the Cryptography Editor QtQuick UI.
 *
 * Contains utility methods to access required objects and data for
 * identity-related cryptography editing.
 */
class CryptographyBackendInterface
{
public:
    virtual ~CryptographyBackendInterface(){};

    /**
     * A list model providing a list of OpenPGP keys.
     */
    virtual QAbstractItemModel *openPgpKeyListModel() const = 0;

    /**
     * A list model providing a list of S/MIME keys.
     */
    virtual QAbstractItemModel *smimeKeyListModel() const = 0;
};

}
}

Q_DECLARE_INTERFACE(KIdentityManagement::Quick::CryptographyBackendInterface, "CryptographyBackendInterface")
