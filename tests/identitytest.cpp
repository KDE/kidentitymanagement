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
  Identity i1 = manager.newFromScratch( "Test" );
  i1.setPrimaryEmailAddress( "firstname.lastname@example.com" );
  i1.setEmailAliases( QStringList() << "firstname@example.com" << "lastname@example.com" );
  QVERIFY( i1.matchesEmailAddress( "\"Lastname, Firstname\" <firstname@example.com>" ) );
  QVERIFY( i1.matchesEmailAddress( "\"Lastname, Firstname\" <firstname.lastname@example.com>" ) );
  QCOMPARE( i1.emailAliases().size(), 2 );
}
