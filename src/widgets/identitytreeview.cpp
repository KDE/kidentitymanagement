// SPDX-FileCopyrightText: 2024-2026 Laurent Montel <montel@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "identitytreeview.h"
#include "identitytreedelegate.h"
#include "identitytreemodel.h"
#include "identitytreesortproxymodel.h"

#include <QHeaderView>

using namespace KIdentityManagementWidgets;
IdentityTreeView::IdentityTreeView(QWidget *parent)
    : QTreeView(parent)
    , mIdentityProxyModel(new KIdentityManagementCore::IdentityTreeSortProxyModel(this))
{
    setAlternatingRowColors(true);
    setSelectionMode(ExtendedSelection);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setRootIsDecorated(false);
    setSortingEnabled(true);
    setAllColumnsShowFocus(true);
    setEditTriggers(QAbstractItemView::EditKeyPressed);
    header()->setSectionsMovable(false);
    header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    mIdentityModel = new KIdentityManagementCore::IdentityTreeModel(KIdentityManagementCore::IdentityManager::self(), this);
    mIdentityModel->setShowDefault(true);

    mIdentityProxyModel->setSourceModel(mIdentityModel);
    setModel(mIdentityProxyModel);

    setColumnHidden(KIdentityManagementCore::IdentityTreeModel::DefaultRole, true);
    setColumnHidden(KIdentityManagementCore::IdentityTreeModel::UoidRole, true);
    setColumnHidden(KIdentityManagementCore::IdentityTreeModel::EmailRole, true);
    setColumnHidden(KIdentityManagementCore::IdentityTreeModel::IdentityNameRole, true);
    setColumnHidden(KIdentityManagementCore::IdentityTreeModel::ActivitiesRole, true);
    setColumnHidden(KIdentityManagementCore::IdentityTreeModel::EnabledActivitiesRole, true);
    setItemDelegateForColumn(KIdentityManagementCore::IdentityTreeModel::DisplayIdentityNameRole, new IdentityTreeDelegate(this));
}

IdentityTreeView::~IdentityTreeView() = default;

KIdentityManagementCore::IdentityActivitiesAbstract *IdentityTreeView::identityActivitiesAbstract() const
{
    return mIdentityProxyModel->identityActivitiesAbstract();
}

bool IdentityTreeView::enablePlasmaActivities() const
{
    return mIdentityProxyModel->enablePlasmaActivities();
}

void IdentityTreeView::setEnablePlasmaActivities(bool newEnablePlasmaActivities)
{
    mIdentityProxyModel->setEnablePlasmaActivities(newEnablePlasmaActivities);
}

KIdentityManagementCore::IdentityTreeSortProxyModel *IdentityTreeView::identityProxyModel() const
{
    return mIdentityProxyModel;
}

void IdentityTreeView::setIdentityActivitiesAbstract(KIdentityManagementCore::IdentityActivitiesAbstract *newIdentityActivitiesAbstract)
{
    mIdentityProxyModel->setIdentityActivitiesAbstract(newIdentityActivitiesAbstract);
}

KIdentityManagementCore::Identity &IdentityTreeView::modifyIdentityForUoid(uint uoid)
{
    return mIdentityModel->identityManager()->modifyIdentityForUoid(uoid);
}

KIdentityManagementCore::IdentityTreeModel *IdentityTreeView::identityTreeModel() const
{
    return mIdentityModel;
}

#include "moc_identitytreeview.cpp"
