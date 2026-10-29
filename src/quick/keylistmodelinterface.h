// SPDX-FileCopyrightText: 2023 Claudio Cambra <claudio.cambra@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include "kidentitymanagementquick_export.h"

#include <QHash>
#include <QObject>

class QModelIndex;

namespace KIdentityManagementCore
{
class Identity;
}
namespace KIdentityManagementQuick
{

class KeyUseTypes
{
    Q_GADGET

public:
    enum class KeyUse {
        KeySigningUse,
        KeyEncryptionUse
    };
    Q_ENUM(KeyUse)
};

/*!
 * \class KIdentityManagementQuick::KeyListModelInterface
 * \inmodule KIdentityManagementQuick
 * \inheaderfile KIdentityManagementQuick/KeyListModelInterface
 *
 * \brief This interface defines the roles and rolenames that are expected
 * in the QtQuick Cryptography editor.
 */
class KIDENTITYMANAGEMENTQUICK_EXPORT KeyListModelInterface
{
public:
    /*!
     * Destroys the key list model interface.
     */
    virtual ~KeyListModelInterface() { };
    /*!
     * \enum KIdentityManagementQuick::KeyListModelInterface::Roles
     *
     * Data roles expected from the key list model.
     *
     * \value KeyIdentifierRole The identifier of the key
     * \value KeyByteArrayRole The key as a byte array
     */
    enum Roles {
        KeyIdentifierRole = Qt::UserRole + 1,
        KeyByteArrayRole
    };

    /*!
     * Returns the role names used by the QtQuick Cryptography editor.
     */
    static QHash<int, QByteArray> roleNames()
    {
        return {{KeyIdentifierRole, "keyIdentifier"}, {KeyByteArrayRole, "keyByteArray"}};
    }

    /*!
     * Returns the index of the key used by \a identity for the given \a keyUse.
     */
    virtual QModelIndex indexForIdentity(const KIdentityManagementCore::Identity &identity, const KeyUseTypes::KeyUse keyUse) const = 0;
};
}

Q_DECLARE_INTERFACE(KIdentityManagementQuick::KeyListModelInterface, "KeyListModelInterface")
