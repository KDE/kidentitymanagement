// SPDX-FileCopyrightText: 2024 Laurent Montel <montel@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
#include "identitytreeviewtest.h"
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
    QCOMPARE(w.selectionMode(), QAbstractItemView::SingleSelection);
    QCOMPARE(w.selectionBehavior(), QAbstractItemView::SelectRows);
    QCOMPARE(w.contextMenuPolicy(), Qt::CustomContextMenu);
}