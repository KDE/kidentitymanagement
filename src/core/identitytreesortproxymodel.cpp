// SPDX-FileCopyrightText: 2024-2025 Laurent Montel <montel@kde.org>
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
    if (mIdentityActivitiesAbstract && mEnablePlasmaActivities) {
        const bool enableActivities = sourceModel()->index(source_row, IdentityTreeModel::EnabledActivitiesRole).data().toBool();
        if (enableActivities) {
            const auto activities = sourceModel()->index(source_row, IdentityTreeModel::ActivitiesRole).data().toStringList();
            const bool result = mIdentityActivitiesAbstract->filterAcceptsRow(activities);
            // qDebug() << " result " << result << " identity name : " << sourceModel()->index(source_row,
            // IdentityTreeModel::IdentityNameRole).data().toString();
            return result;
        }
    }
    return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}

bool IdentityTreeSortProxyModel::enablePlasmaActivities() const
{
    return mEnablePlasmaActivities;
}

void IdentityTreeSortProxyModel::setEnablePlasmaActivities(bool newEnablePlasmaActivities)
{
    if (mEnablePlasmaActivities != newEnablePlasmaActivities) {
        mEnablePlasmaActivities = newEnablePlasmaActivities;
        invalidateFilter();
    }
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

#include "moc_identitytreesortproxymodel.cpp"
