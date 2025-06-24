// SPDX-FileCopyrightText: 2023 Claudio Cambra <claudio.cambra@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "identitymodeltest.h"
using namespace Qt::Literals::StringLiterals;

#include <QStandardPaths>
#include <QTest>

#include <KIdentityManagementCore/Identity>
#include <KIdentityManagementCore/IdentityModel>

using namespace KIdentityManagementCore;

namespace
{
const auto i1Name = u"Test1"_s;
const auto i1Email = u"firstname.lastname@example.com"_s;

void cleanupIdentities(std::unique_ptr<IdentityManager> &manager)
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
    manager = std::make_unique<IdentityManager>();

    cleanupIdentities(manager);
    QCOMPARE(manager->identities().count(), 1); // Can't remove all identities

    {
        auto &i1 = manager->newFromScratch(i1Name);
        i1.setPrimaryEmailAddress(i1Email);
        i1.setEmailAliases(QStringList{u"firstname@example.com"_s, u"lastname@example.com"_s});
    }

    {
        auto &i2 = manager->newFromScratch(u"Test2"_s);
        i2.setPrimaryEmailAddress(u"test@test.de"_s);
    }

    // Remove the first identity, which we couldn't remove above
    QVERIFY(manager->removeIdentity(manager->identities().at(0)));

    manager->commit();
    QCOMPARE(manager->identities().count(), 2);
}

void IdentityModelTester::testModelCount()
{
    IdentityModel model;
    QCOMPARE(model.rowCount(), 2);
}

void IdentityModelTester::testModelData()
{
    IdentityModel model;
    const auto &i1 = IdentityManager::self()->modifyIdentityForName(i1Name);
    const auto i1Index = model.index(0, 0);
    QCOMPARE(i1Index.data(IdentityModel::IdentityNameRole), i1Name);
    QCOMPARE(i1Index.data(IdentityModel::EmailRole), i1Email);
    QCOMPARE(i1Index.data(IdentityModel::UoidRole).toUInt(), i1.uoid());
}

void IdentityModelTester::testEmailFromUoid()
{
    IdentityModel model;
    const auto &i1 = IdentityManager::self()->modifyIdentityForName(i1Name);
    QCOMPARE(model.email(i1.uoid()), i1Email);
}

#include "moc_identitymodeltest.cpp"
