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
#include "qtest_kde.h"
#include "identitytest.h"
#include "../identitymanager.h"
#include "../identity.h"

#include <KConfig>
#include <KConfigGroup>

using namespace KPIMIdentities;

QTEST_KDEMAIN_CORE( IdentityTester )

void IdentityTester::test_NullIdentity()
{
  IdentityManager manager;
  QVERIFY( manager.identityForAddress( "thisaddressforsuredoesnotexist@kde.org" ).isNull() );
}

void IdentityTester::test_Aliases()
{
  IdentityManager manager;

  // It is picking up identities from older tests somethimes, so cleanup
  while ( manager.identities().size() > 1 ) {
    manager.removeIdentity( manager.identities().first() );
    manager.commit();
  }

  Identity &i1 = manager.newFromScratch( "Test1" );
  i1.setPrimaryEmailAddress( "firstname.lastname@example.com" );
  i1.setEmailAliases( QStringList() << "firstname@example.com" << "lastname@example.com" );
  QVERIFY( i1.matchesEmailAddress( "\"Lastname, Firstname\" <firstname@example.com>" ) );
  QVERIFY( i1.matchesEmailAddress( "\"Lastname, Firstname\" <firstname.lastname@example.com>" ) );
  QCOMPARE( i1.emailAliases().size(), 2 );

  KConfig testConfig("test");
  KConfigGroup testGroup( &testConfig, "testGroup" );
  i1.writeConfig( testGroup );
  i1.readConfig( testGroup );
  QCOMPARE( i1.emailAliases().size(), 2 );

  manager.commit();

  Identity &i2 = manager.newFromScratch( "Test2" );
  i2.setPrimaryEmailAddress( "test@test.de" );
  QVERIFY( i2.emailAliases().isEmpty() );
  manager.commit();

  // Remove the first identity, which we couldn't remove above
  manager.removeIdentity( manager.identities().first() );
  manager.commit();

  QCOMPARE( manager.allEmails().size(), 4 );
  QCOMPARE( manager.identityForAddress( "firstname@example.com" ).identityName().toAscii().data(), "Test1" );
}
