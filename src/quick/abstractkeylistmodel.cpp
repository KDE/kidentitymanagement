// SPDX-FileCopyrightText: 2023 Claudio Cambra <claudio.cambra@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "abstractkeylistmodel.h"

namespace KIdentityManagement
{
namespace Quick
{

AbstractKeyListModel::AbstractKeyListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

QHash<int, QByteArray> AbstractKeyListModel::roleNames() const
{
    auto existing = QAbstractListModel::roleNames();
    existing.insert({{KeyIdentifierRole, "keyIdentifier"}, {KeyByteArrayRole, "keyByteArray"}});
    return existing;
}
}
}
