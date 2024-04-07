// SPDX-FileCopyrightText: 2024 Laurent Montel <montel@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "identitytreesortproxymodel.h"

using namespace KIdentityManagementWidgets;
IdentityTreeSortProxyModel::IdentityTreeSortProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

IdentityTreeSortProxyModel::~IdentityTreeSortProxyModel() = default;

bool IdentityTreeSortProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    // TODO
    return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}
