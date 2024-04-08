// SPDX-FileCopyrightText: 2024 Laurent Montel <montel@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "identitytreeview.h"
#include "identitytreemodel.h"

using namespace KIdentityManagementWidgets;
IdentityTreeView::IdentityTreeView(QWidget *parent)
    : QTreeView(parent)
{
    setAlternatingRowColors(true);
    setSelectionMode(SingleSelection);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setRootIsDecorated(false);
    setSortingEnabled(true);
    setModel(new IdentityTreeModel(this));

    setColumnHidden(IdentityTreeModel::DefaultRole, true);
    setColumnHidden(IdentityTreeModel::UoidRole, true);
}

IdentityTreeView::~IdentityTreeView() = default;

#include "moc_identitytreeview.cpp"
