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

#include <KIconLoader>
#include <KStandardDirs>
#include <KConfigGroup>

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

static void setCursorPos( QTextEdit &edit, int pos )
{
  QTextCursor cursor( edit.document() );
  cursor.setPosition( pos );
  edit.setTextCursor( cursor );
}

void SignatureTester::testTextEditInsertion()
{
  TextEdit edit;
  Signature sig;
  sig.setText( "Hello World" );

  // Test inserting signature at start, with seperators
  edit.setPlainText( "Bla Bla" );
  sig.insertIntoTextEdit( &edit, Signature::Start, true, true );
  QVERIFY( edit.textMode() == KRichTextEdit::Plain );
  QCOMPARE( edit.toPlainText(), QString( "\n-- \nHello World\nBla Bla" ) );

  // Test inserting signature at end. make sure cursor position is preserved
  edit.clear();
  edit.setPlainText( "Bla Bla" );
  setCursorPos( edit, 4 );
  sig.insertIntoTextEdit( &edit, Signature::End, true, true );
  QCOMPARE( edit.toPlainText(), QString( "Bla Bla\n-- \nHello World" ) );
  QCOMPARE( edit.textCursor().position(), 4 );

  // test inserting a signature at cursor position. make sure the cursor
  // moves the position correctly. make sure modified state is preserved
  edit.clear();
  edit.setPlainText( "Bla Bla" );
  setCursorPos( edit, 4 );
  edit.document()->setModified( false );
  sig.insertIntoTextEdit( &edit, Signature::AtCursor, true, true );
  QCOMPARE( edit.toPlainText(), QString( "Bla \n-- \nHello World\nBla" ) );
  QCOMPARE( edit.textCursor().position(), 21 );
  QVERIFY( !edit.document()->isModified() );

  // make sure undo undoes everything in one go
  edit.undo();
  QCOMPARE( edit.toPlainText(), QString( "Bla Bla" ) );

  // test inserting signature without seperator.
  // make sure cursor position and modified state is preserved.
  edit.clear();
  edit.setPlainText( "Bla Bla" );
  setCursorPos( edit, 4 );
  edit.document()->setModified( true );
  sig.insertIntoTextEdit( &edit, Signature::End, false, true );
  QCOMPARE( edit.toPlainText(), QString( "Bla Bla\nHello World" ) );
  QCOMPARE( edit.textCursor().position(), 4 );
  QVERIFY( edit.document()->isModified() );

  sig.setText( "Hello<br>World" );
  sig.setInlinedHtml( true );

  // test that html signatures turn html on and have correct line endings (<br> vs \n)
  edit.clear();
  edit.setPlainText( "Bla Bla" );
  sig.insertIntoTextEdit( &edit, Signature::End, true, true );
  QVERIFY( edit.textMode() == KRichTextEdit::Rich );
  QCOMPARE( edit.toPlainText(), QString( "Bla Bla\n-- \nHello\nWorld" ) );
}

void SignatureTester::testBug167961()
{
  TextEdit edit;
  Signature sig;
  sig.setText( "BLA" );

  // Test that the cursor is still at the start when appending a sig into
  // an empty text edit
  sig.insertIntoTextEdit( &edit, Signature::End, true, true );
  QCOMPARE( edit.textCursor().position(), 0 );

  // OTOH, when prepending a sig, the cursor should be at the end
  edit.clear();
  sig.insertIntoTextEdit( &edit, Signature::Start, true, true );
  QCOMPARE( edit.textCursor().position(), 9 ); // "\n-- \nBLA\n"
}

// Make writeConfig() public, we need it
class MySignature : public Signature
{
  public:
    using Signature::writeConfig;
    using Signature::readConfig;
};

void SignatureTester::testImages()
{
  TextEdit edit;
  QString image1Path = KIconLoader::global()->iconPath( QLatin1String( "folder-new" ), KIconLoader::Small, false );
  QString image2Path = KIconLoader::global()->iconPath( QLatin1String( "arrow-up" ), KIconLoader::Small, false );
  QImage image1, image2;
  QVERIFY( image1.load( image1Path ) );
  QVERIFY( image2.load( image1Path ) );
  QString path = KStandardDirs::locateLocal( "data", "emailidentities/unittest/" );
  QString configPath = KStandardDirs::locateLocal( "config", "signaturetest" );
  KConfig config( configPath );
  KConfigGroup group1 = config.group( "Signature1" );

  MySignature sig;
  sig.setImageLocation( path );
  sig.setInlinedHtml( true );
  sig.setText( "Bla<img src=\"folder-new.png\">Bla" );
  sig.addImage( image1, "folder-new.png" );
  sig.writeConfig( group1 );

  // OK, signature saved, the image should be saved as well
  QDir dir( path );
  QStringList entryList = dir.entryList( QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks );
  QCOMPARE( entryList.count(), 1 );
  QCOMPARE( entryList.first(), QString( "folder-new.png" ) );

  // Now, set the text of the signature to something without images, then save it.
  // The signature class should have removed the images.
  sig.setText( "ascii ribbon campaign - against html mail" );
  sig.writeConfig( group1 );
  entryList = dir.entryList( QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks );
  QCOMPARE( entryList.count(), 0 );

  // Enable images again, this time with two of the buggers
  sig.setText( "Bla<img src=\"folder-new.png\">Bla<img src=\"arrow-up.png\">Bla" );
  sig.addImage( image1, "folder-new.png" );
  sig.addImage( image2, "arrow-up.png" );
  sig.writeConfig( group1 );
  entryList = dir.entryList( QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks );
  QCOMPARE( entryList.count(), 2 );
  QCOMPARE( entryList.first(), QString( "arrow-up.png" ) );
  QCOMPARE( entryList.last(), QString( "folder-new.png" ) );

  // Now, create a second signature instance from the same config, and make sure it can still
  // read the images, and it does not mess up
  MySignature sig2;
  sig2.readConfig( group1 );
  sig2.insertIntoTextEdit( &edit, KPIMIdentities::Signature::End, true, true );
  QCOMPARE( edit.embeddedImages().count(), 2 );
  QCOMPARE( sig2.text(), QString( "Bla<img src=\"folder-new.png\">Bla<img src=\"arrow-up.png\">Bla") );
  sig2.writeConfig( group1 );
  entryList = dir.entryList( QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks );
  QCOMPARE( entryList.count(), 2 );
  QCOMPARE( entryList.first(), QString( "arrow-up.png" ) );
  QCOMPARE( entryList.last(), QString( "folder-new.png" ) );

  // Remove one image from the signature, and make sure only 1 file is left one file system.
  sig2.setText( "<img src=\"folder-new.png\">" );
  sig2.writeConfig( group1 );
  edit.clear();
  sig2.insertIntoTextEdit( &edit, Signature::End, true, true );
  QCOMPARE( edit.embeddedImages().size(), 1 );
  entryList = dir.entryList( QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks );
  QCOMPARE( entryList.count(), 1 );
}

void SignatureTester::testLinebreaks()
{
  Signature sig;
  sig.setType( Signature::Inlined );
  sig.setInlinedHtml( true );
  sig.setText( "Hans Mustermann<br>Musterstr. 42" );

  KPIMTextEdit::TextEdit edit;
  sig.insertIntoTextEdit( &edit, Signature::Start, false, false );
  QCOMPARE( edit.toPlainText(), QString( "Hans Mustermann\nMusterstr. 42" ) );

  sig.setText( "<p>Hans Mustermann</p><br>Musterstr. 42" );
  sig.insertIntoTextEdit( &edit, Signature::Start, true, false );
  QCOMPARE( edit.toPlainText(), QString( "-- \nHans Mustermann\nMusterstr. 42" ) );
}

