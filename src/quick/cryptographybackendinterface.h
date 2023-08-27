// SPDX-FileCopyrightText: 2023 Claudio Cambra <claudio.cambra@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include "kidentitymanagementquick_export.h"

#include <QObject>
#include <memory>

class QAbstractItemModel;

namespace KIdentityManagement
{
namespace Quick
{

/**
 * @brief Defines the interface used by the Cryptography Editor Backend.
 *
 * This class is held by the CryptographyBackend, which exposes what is
 * needed to the Cryptography Editor QtQuick UI.
 *
 * Contains utility methods to access required objects and data for
 * identity-related cryptography editing.
 */
class KIDENTITYMANAGEMENTQUICK_EXPORT CryptographyBackendInterface
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

using CryptographyBackendInterfacePtr = std::shared_ptr<CryptographyBackendInterface>;
}
}

Q_DECLARE_INTERFACE(KIdentityManagement::Quick::CryptographyBackendInterface, "CryptographyBackendInterface")
