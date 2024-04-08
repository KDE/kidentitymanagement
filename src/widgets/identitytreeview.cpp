// SPDX-FileCopyrightText: 2024 Laurent Montel <montel@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "identitytreeview.h"
#include "identitytreemodel.h"
#include "identitytreesortproxymodel.h"

#include <QHeaderView>

using namespace KIdentityManagementWidgets;
IdentityTreeView::IdentityTreeView(QWidget *parent)
    : QTreeView(parent)
    , mIdentityProxyModel(new IdentityTreeSortProxyModel(this))
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

    auto model = new IdentityTreeModel(this);
    model->setShowDefault(true);

    mIdentityProxyModel->setSourceModel(model);
    setModel(mIdentityProxyModel);

    setColumnHidden(IdentityTreeModel::DefaultRole, true);
    setColumnHidden(IdentityTreeModel::UoidRole, true);
    setColumnHidden(IdentityTreeModel::EmailRole, true);
}

IdentityTreeView::~IdentityTreeView() = default;

#include "moc_identitytreeview.cpp"
