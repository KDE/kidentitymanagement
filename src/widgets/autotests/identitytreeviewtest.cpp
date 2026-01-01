// SPDX-FileCopyrightText: 2024-2026 Laurent Montel <montel@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
#include "identitytreeviewtest.h"
#include "identitytreemodel.h"
#include "identitytreeview.h"
#include <QTest>
QTEST_MAIN(IdentityTreeViewTest)
IdentityTreeViewTest::IdentityTreeViewTest(QObject *parent)
    : QObject{parent}
{
}

void IdentityTreeViewTest::shouldHaveDefaultValues()
{
    KIdentityManagementWidgets::IdentityTreeView w;
    QVERIFY(w.alternatingRowColors());
    QVERIFY(!w.rootIsDecorated());
    QVERIFY(w.isSortingEnabled());
    QCOMPARE(w.selectionMode(), QAbstractItemView::ExtendedSelection);
    QCOMPARE(w.selectionBehavior(), QAbstractItemView::SelectRows);
    QCOMPARE(w.contextMenuPolicy(), Qt::CustomContextMenu);
    QVERIFY(!w.identityActivitiesAbstract());
    QVERIFY(w.isColumnHidden(KIdentityManagementCore::IdentityTreeModel::DefaultRole));
    QVERIFY(w.isColumnHidden(KIdentityManagementCore::IdentityTreeModel::UoidRole));
    QVERIFY(w.isColumnHidden(KIdentityManagementCore::IdentityTreeModel::IdentityNameRole));
    QVERIFY(w.isColumnHidden(KIdentityManagementCore::IdentityTreeModel::EmailRole));
    QVERIFY(w.isColumnHidden(KIdentityManagementCore::IdentityTreeModel::ActivitiesRole));
}

#include "moc_identitytreeviewtest.cpp"
