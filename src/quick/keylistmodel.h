// SPDX-FileCopyrightText: 2023 Claudio Cambra <claudio.cambra@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QAbstractListModel>
#include <QHash>

namespace KIdentityManagement
{
namespace Quick
{

/**
 * This namespace defines the roles and rolenames that are expected
 * in the QtQuick Cryptography editor. Implement these in your chosen
 * backing model!
 */
namespace KeyListModel
{
enum Roles { KeyIdentifierRole = Qt::UserRole + 1, KeyByteArrayRole };
static const QHash<int, QByteArray> roleNames = {{KeyIdentifierRole, "keyIdentifier"}, {KeyByteArrayRole, "keyByteArray"}};
}

}
}
