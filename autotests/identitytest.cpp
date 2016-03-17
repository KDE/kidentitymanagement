/*
    Copyright (c) 2008 Thomas McGuire <mcguire@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/
#include "qtest.h"
#include "identitytest.h"
#include "identitymanager.h"
#include "identity.h"

#include <KConfig>
#include <KConfigGroup>

#include <QMimeData>
#include <QStandardPaths>

using namespace KIdentityManagement;

QTEST_GUILESS_MAIN(IdentityTester)

void IdentityTester::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void IdentityTester::test_NullIdentity()
{
    IdentityManager manager;
    QVERIFY(manager.identityForAddress(QStringLiteral("thisaddressforsuredoesnotexist@kde.org")).isNull());
}

void IdentityTester::test_Aliases()
{
    IdentityManager manager;

    // It is picking up identities from older tests somethimes, so cleanup
    while (manager.identities().size() > 1) {
        manager.removeIdentity(manager.identities().at(0));
        manager.commit();
    }

    Identity &i1 = manager.newFromScratch(QStringLiteral("Test1"));
    i1.setPrimaryEmailAddress(QStringLiteral("firstname.lastname@example.com"));
    i1.setEmailAliases(QStringList() << QStringLiteral("firstname@example.com") << QStringLiteral("lastname@example.com"));
    QVERIFY(i1.matchesEmailAddress(QStringLiteral("\"Lastname, Firstname\" <firstname@example.com>")));
    QVERIFY(i1.matchesEmailAddress(QStringLiteral("\"Lastname, Firstname\" <firstname.lastname@example.com>")));
    QCOMPARE(i1.emailAliases().size(), 2);

    KConfig testConfig(QStringLiteral("test"));
    KConfigGroup testGroup(&testConfig, "testGroup");
    i1.writeConfig(testGroup);
    i1.readConfig(testGroup);
    QCOMPARE(i1.emailAliases().size(), 2);

    manager.commit();

    Identity &i2 = manager.newFromScratch(QStringLiteral("Test2"));
    i2.setPrimaryEmailAddress(QStringLiteral("test@test.de"));
    QVERIFY(i2.emailAliases().isEmpty());
    manager.commit();

    // Remove the first identity, which we couldn't remove above
    manager.removeIdentity(manager.identities().at(0));
    manager.commit();

    QCOMPARE(manager.allEmails().size(), 4);
    QCOMPARE(manager.identityForAddress(QStringLiteral("firstname@example.com")).identityName().toLatin1().data(), "Test1");
}

void IdentityTester::test_toMimeData()
{
    IdentityManager manager;
    Identity &identity = manager.newFromScratch(QStringLiteral("Test1"));
    identity.setFullName(QStringLiteral("name"));
    QMimeData mimeData;
    identity.populateMimeData(&mimeData);

    Identity identity2 = Identity::fromMimeData(&mimeData);

    // The deserializer fills in the QHash will lots of invalid variants, which is OK, but the CTOR doesn't fill
    // The hash with the missing fields, so this comparison will fail. Maybe do a smarter Identity== where missing
    // from the hash or being invalid is equivalent
    QEXPECT_FAIL("", "The deserialized instance is different", Continue);
    QCOMPARE(identity, identity2);

    QEXPECT_FAIL("", "Missing qRegisterMetaTypeStreamOperators() I guess ?", Continue);
    QCOMPARE(identity.fullName(), identity2.fullName());
}
