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
 * \brief This interface defines the roles and rolenames that are expected
 * in the QtQuick Cryptography editor.
 */
class KIDENTITYMANAGEMENTQUICK_EXPORT KeyListModelInterface
{
public:
    virtual ~KeyListModelInterface() { };
    enum Roles {
        KeyIdentifierRole = Qt::UserRole + 1,
        KeyByteArrayRole
    };

    static QHash<int, QByteArray> roleNames()
    {
        return {{KeyIdentifierRole, "keyIdentifier"}, {KeyByteArrayRole, "keyByteArray"}};
    }

    virtual QModelIndex indexForIdentity(const KIdentityManagementCore::Identity &identity, const KeyUseTypes::KeyUse keyUse) const = 0;
};
}

Q_DECLARE_INTERFACE(KIdentityManagementQuick::KeyListModelInterface, "KeyListModelInterface")
