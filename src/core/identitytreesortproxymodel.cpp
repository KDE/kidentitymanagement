// SPDX-FileCopyrightText: 2024 Laurent Montel <montel@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "identitytreesortproxymodel.h"
#include "identityactivitiesabstract.h"
#include "identitytreemodel.h"
using namespace KIdentityManagementCore;
IdentityTreeSortProxyModel::IdentityTreeSortProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

IdentityTreeSortProxyModel::~IdentityTreeSortProxyModel() = default;

bool IdentityTreeSortProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if (mIdentityActivitiesAbstract) {
        const auto activities = sourceModel()->index(source_row, 0).data(IdentityTreeModel::ActivitiesRole).toStringList();
        return mIdentityActivitiesAbstract->filterAcceptsRow(activities);
    }
    return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}

IdentityActivitiesAbstract *IdentityTreeSortProxyModel::identityActivitiesAbstract() const
{
    return mIdentityActivitiesAbstract;
}

void IdentityTreeSortProxyModel::setIdentityActivitiesAbstract(IdentityActivitiesAbstract *newIdentityActivitiesAbstract)
{
    if (mIdentityActivitiesAbstract != newIdentityActivitiesAbstract) {
        mIdentityActivitiesAbstract = newIdentityActivitiesAbstract;
        connect(mIdentityActivitiesAbstract, &IdentityActivitiesAbstract::activitiesChanged, this, &IdentityTreeSortProxyModel::invalidateFilter);
        invalidateFilter();
    }
}
