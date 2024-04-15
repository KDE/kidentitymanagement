// SPDX-FileCopyrightText: 2024 Laurent Montel <montel@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "identitytreesortproxymodel.h"
#include "identityactivitiesabstract.h"
using namespace KIdentityManagementCore;
IdentityTreeSortProxyModel::IdentityTreeSortProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

IdentityTreeSortProxyModel::~IdentityTreeSortProxyModel() = default;

bool IdentityTreeSortProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if (mIdentityActivitiesAbstract) {
        return mIdentityActivitiesAbstract->filterAcceptsRow(source_row, source_parent);
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
