// SPDX-FileCopyrightText: 2024 Laurent Montel <montel@kde.org>
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
    setSelectionMode(SingleSelection);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setRootIsDecorated(false);
    setSortingEnabled(true);
    setAllColumnsShowFocus(true);
    header()->setSectionsMovable(false);
    header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    auto model = new KIdentityManagementCore::IdentityTreeModel(KIdentityManagementCore::IdentityManager::self(), this);
    model->setShowDefault(true);

    mIdentityProxyModel->setSourceModel(model);
    setModel(mIdentityProxyModel);

    setColumnHidden(KIdentityManagementCore::IdentityTreeModel::DefaultRole, true);
    setColumnHidden(KIdentityManagementCore::IdentityTreeModel::UoidRole, true);
    setColumnHidden(KIdentityManagementCore::IdentityTreeModel::EmailRole, true);
    setColumnHidden(KIdentityManagementCore::IdentityTreeModel::IdentityNameRole, true);
    setColumnHidden(KIdentityManagementCore::IdentityTreeModel::ActivitiesRole, true);
    setItemDelegateForColumn(KIdentityManagementCore::IdentityTreeModel::DisplayIdentityNameRole, new IdentityTreeDelegate(this));
}

IdentityTreeView::~IdentityTreeView() = default;

KIdentityManagementCore::IdentityActivitiesAbstract *IdentityTreeView::identityActivitiesAbstract() const
{
    return mIdentityProxyModel->identityActivitiesAbstract();
}

void IdentityTreeView::setIdentityActivitiesAbstract(KIdentityManagementCore::IdentityActivitiesAbstract *newIdentityActivitiesAbstract)
{
    mIdentityProxyModel->setIdentityActivitiesAbstract(newIdentityActivitiesAbstract);
}

#include "moc_identitytreeview.cpp"
