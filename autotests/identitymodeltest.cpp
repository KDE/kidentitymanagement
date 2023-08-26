// SPDX-FileCopyrightText: 2023 Claudio Cambra <claudio.cambra@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "identitymodeltest.h"

#include <QStandardPaths>
#include <QTest>

#include "identity.h"
#include "identitymanager.h"

using namespace KIdentityManagement;

namespace
{
void cleanupIdentities(IdentityManager *const manager)
{
    QVERIFY(manager);
    // It is picking up identities from older tests sometimes, so cleanup
    // Note this will always leave at least one identity -- remove later
    while (manager->identities().count() > 1) {
        QVERIFY(manager->removeIdentity(manager->identities().at(0)));
        manager->commit();
    }
}
}

QTEST_GUILESS_MAIN(IdentityModelTester)

void IdentityModelTester::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);

    IdentityManager manager;

    cleanupIdentities(&manager);
    QCOMPARE(manager.identities().count(), 1); // Can't remove all identities

    {
        auto &i1 = manager.newFromScratch(QStringLiteral("Test1"));
        i1.setPrimaryEmailAddress(QStringLiteral("firstname.lastname@example.com"));
        i1.setEmailAliases({QStringLiteral("firstname@example.com"), QStringLiteral("lastname@example.com")});
    }

    {
        auto &i2 = manager.newFromScratch(QStringLiteral("Test2"));
        i2.setPrimaryEmailAddress(QStringLiteral("test@test.de"));
    }

    // Remove the first identity, which we couldn't remove above
    QVERIFY(manager.removeIdentity(manager.identities().at(0)));

    manager.commit();
    QCOMPARE(manager.identities().count(), 2);
}
