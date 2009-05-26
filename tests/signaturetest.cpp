/*
    Copyright (c) 20089 Thomas McGuire <mcguire@kde.org>

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
#include "signaturetest.h"

#include "../signature.h"

#include "kpimtextedit/textedit.h"

using namespace KPIMIdentities;
using namespace KPIMTextEdit;

QTEST_KDEMAIN( SignatureTester, GUI )

void SignatureTester::testSignatures()
{
  Signature sig1;
  sig1.setText( "Hello World" );
  QCOMPARE( sig1.text(), QString( "Hello World" ) );
  QCOMPARE( sig1.type(), Signature::Inlined );
  QCOMPARE( sig1.rawText(), QString( "Hello World" ) );
  QVERIFY( !sig1.isInlinedHtml() );
  QCOMPARE( sig1.withSeparator(), QString( "-- \nHello World" ) );

  Signature sig2;
  sig2.setText( "<b>Hello</b> World" );
  sig2.setInlinedHtml( true );
  QVERIFY( sig2.isInlinedHtml() );
  QCOMPARE( sig2.type(), Signature::Inlined );
  QCOMPARE( sig2.rawText(), QString( "<b>Hello</b> World" ) );
  QCOMPARE( sig2.withSeparator(), QString( "-- <br><b>Hello</b> World" ) );

  // Read this very file in, we use it for the tests
  QFile thisFile( __FILE__ );
  thisFile.open( QIODevice::ReadOnly );
  QString fileContent = QString::fromUtf8( thisFile.readAll() );

  Signature sig3;
  sig3.setUrl( QString( "cat " ) + QString( __FILE__ ), true );
  QCOMPARE( sig3.rawText(), fileContent );
  QVERIFY( !sig3.isInlinedHtml() );
  QVERIFY( sig3.text().isEmpty() );
  QCOMPARE( sig3.type(), Signature::FromCommand );
  QCOMPARE( sig3.withSeparator(), QString( "-- \n" ) + fileContent );

  Signature sig4;
  sig4.setUrl( __FILE__, false );
  QCOMPARE( sig4.rawText(), fileContent );
  QVERIFY( !sig4.isInlinedHtml() );
  QVERIFY( sig4.text().isEmpty() );
  QCOMPARE( sig4.type(), Signature::FromFile );
  QCOMPARE( sig4.withSeparator(), QString( "-- \n" ) + fileContent );

}

