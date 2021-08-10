/*
    SPDX-FileCopyrightText: 2016-2021 Laurent Montel <montel@kde.org>
    SPDX-FileCopyrightText: 2008 Thomas McGuire <mcguire@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#undef QT_USE_FAST_CONCATENATION
#undef QT_USE_FAST_OPERATOR_PLUS

#include "signaturetest.h"
#include "qtest.h"

#include "signature.h"

#include <KActionCollection>
#include <KConfig>
#include <KConfigGroup>
#include <KShell>
#include <QDir>
#include <QStandardPaths>

#include <kpimtextedit/richtextcomposer.h>
#include <kpimtextedit/richtextcomposercontroler.h>
#include <kpimtextedit/richtextcomposerimages.h>
using namespace KIdentityManagement;
using namespace KPIMTextEdit;

QTEST_MAIN(SignatureTester)

void SignatureTester::testEqualSignatures()
{
    Signature sig1;
    sig1.setText(QStringLiteral("Hello World"));
    sig1.setEnabledSignature(true);
    Signature sig2(sig1);
    QVERIFY(sig1 == sig2);
    QCOMPARE(sig2.text(), QStringLiteral("Hello World"));
    QCOMPARE(sig2.type(), Signature::Inlined);
    QCOMPARE(sig2.rawText(), QStringLiteral("Hello World"));
    QVERIFY(!sig2.isInlinedHtml());
    QCOMPARE(sig2.withSeparator(), QStringLiteral("-- \nHello World"));
    QVERIFY(sig2.isEnabledSignature());

    Signature sig3 = sig1;
    QVERIFY(sig1 == sig3);
    QCOMPARE(sig3.text(), QStringLiteral("Hello World"));
    QCOMPARE(sig3.type(), Signature::Inlined);
    QCOMPARE(sig3.rawText(), QStringLiteral("Hello World"));
    QVERIFY(!sig3.isInlinedHtml());
    QCOMPARE(sig3.withSeparator(), QStringLiteral("-- \nHello World"));
    QVERIFY(sig3.isEnabledSignature());
}

void SignatureTester::testSignatures()
{
    Signature sig1;
    sig1.setText(QStringLiteral("Hello World"));
    QCOMPARE(sig1.text(), QStringLiteral("Hello World"));
    QCOMPARE(sig1.type(), Signature::Inlined);
    QCOMPARE(sig1.rawText(), QStringLiteral("Hello World"));
    QVERIFY(!sig1.isInlinedHtml());
    QCOMPARE(sig1.withSeparator(), QStringLiteral("-- \nHello World"));
    QVERIFY(!sig1.isEnabledSignature());

    Signature sig2;
    sig2.setText(QStringLiteral("<b>Hello</b> World"));
    sig2.setInlinedHtml(true);
    QVERIFY(sig2.isInlinedHtml());
    QCOMPARE(sig2.type(), Signature::Inlined);
    QCOMPARE(sig2.rawText(), QStringLiteral("<b>Hello</b> World"));
    QCOMPARE(sig2.withSeparator(), QStringLiteral("-- <br><b>Hello</b> World"));
    QVERIFY(!sig2.isEnabledSignature());

    const QString dataFilePath = QStringLiteral(SIGNATURETEST_DATA_FILE);
    // Read this very file in, we use it for the tests
    QFile thisFile(dataFilePath);
    thisFile.open(QIODevice::ReadOnly);
    QString fileContent = QString::fromUtf8(thisFile.readAll());

    if (!QStandardPaths::findExecutable(QStringLiteral("cat")).isEmpty()) {
        Signature sig3;
        sig3.setPath(QStringLiteral("cat ") + KShell::quoteArg(dataFilePath), true);
        QCOMPARE(sig3.rawText(), fileContent);
        QVERIFY(!sig3.isInlinedHtml());
        QVERIFY(sig3.text().isEmpty());
        QCOMPARE(sig3.type(), Signature::FromCommand);
        QCOMPARE(sig3.withSeparator(), QString(QStringLiteral("-- \n") + fileContent));
        QVERIFY(!sig3.isEnabledSignature());
    }

    Signature sig4;
    sig4.setPath(dataFilePath, false);
    QCOMPARE(sig4.rawText(), fileContent);
    QVERIFY(!sig4.isInlinedHtml());
    QVERIFY(sig4.text().isEmpty());
    QCOMPARE(sig4.type(), Signature::FromFile);
    QCOMPARE(sig4.withSeparator(), QString(QStringLiteral("-- \n") + fileContent));
    QVERIFY(!sig4.isEnabledSignature());
}

static void setCursorPos(QTextEdit &edit, int pos)
{
    QTextCursor cursor(edit.document());
    cursor.setPosition(pos);
    edit.setTextCursor(cursor);
}

void SignatureTester::testTextEditInsertion()
{
    KPIMTextEdit::RichTextComposer edit;
    edit.createActions(new KActionCollection(this));
    Signature sig;
    sig.setEnabledSignature(true);
    sig.setText(QStringLiteral("Hello World"));

    // Test inserting signature at start, with separators. Make sure two new
    // lines are inserted before the signature

    edit.setPlainText(QStringLiteral("Bla Bla"));
    sig.insertIntoTextEdit(Signature::Start, Signature::AddSeparator | Signature::AddNewLines, &edit);
    QVERIFY(edit.textMode() == KPIMTextEdit::RichTextComposer::Plain);
    QCOMPARE(edit.toPlainText(), QStringLiteral("\n\n-- \nHello World\nBla Bla"));

    // Test inserting signature at end. make sure cursor position is preserved
    edit.clear();
    edit.setPlainText(QStringLiteral("Bla Bla"));
    setCursorPos(edit, 4);
    sig.insertIntoTextEdit(Signature::End, Signature::AddSeparator | Signature::AddNewLines, &edit);
    QCOMPARE(edit.toPlainText(), QStringLiteral("Bla Bla\n-- \nHello World"));
    QCOMPARE(edit.textCursor().position(), 4);

    // test inserting a signature at cursor position. make sure the signature is inserted
    // to the beginning of the line and make sure modified state is preserved
    edit.clear();
    edit.setPlainText(QStringLiteral("Bla Bla"));
    setCursorPos(edit, 4);
    edit.document()->setModified(false);
    sig.insertIntoTextEdit(Signature::AtCursor, Signature::AddSeparator | Signature::AddNewLines, &edit);
    QCOMPARE(edit.toPlainText(), QStringLiteral("-- \nHello World\nBla Bla"));
    QCOMPARE(edit.textCursor().position(), 20);
    QVERIFY(!edit.document()->isModified());

    // make sure undo undoes everything in one go
    edit.undo();
    QCOMPARE(edit.toPlainText(), QStringLiteral("Bla Bla"));

    // test inserting signature without separator.
    // make sure cursor position and modified state is preserved.
    edit.clear();
    edit.setPlainText(QStringLiteral("Bla Bla"));
    setCursorPos(edit, 4);
    edit.document()->setModified(true);
    sig.insertIntoTextEdit(Signature::End, Signature::AddNewLines, &edit);
    QCOMPARE(edit.toPlainText(), QStringLiteral("Bla Bla\nHello World"));
    QCOMPARE(edit.textCursor().position(), 4);
    QVERIFY(edit.document()->isModified());

    sig.setText(QStringLiteral("Hello<br>World"));
    sig.setInlinedHtml(true);

    // test that html signatures turn html on and have correct line endings (<br> vs \n)
    edit.clear();
    edit.setPlainText(QStringLiteral("Bla Bla"));
    sig.insertIntoTextEdit(Signature::End, Signature::AddSeparator | Signature::AddNewLines, &edit);
    QVERIFY(edit.textMode() == KPIMTextEdit::RichTextComposer::Rich);
    QCOMPARE(edit.toPlainText(), QStringLiteral("Bla Bla\n-- \nHello\nWorld"));
}

void SignatureTester::testBug167961()
{
    KPIMTextEdit::RichTextComposer edit;
    edit.createActions(new KActionCollection(this));
    Signature sig;
    sig.setEnabledSignature(true);
    sig.setText(QStringLiteral("BLA"));

    // Test that the cursor is still at the start when appending a sig into
    // an empty text edit
    sig.insertIntoTextEdit(Signature::End, Signature::AddSeparator | Signature::AddNewLines, &edit);
    QCOMPARE(edit.textCursor().position(), 0);

    // When prepending a sig, the cursor should also be at the start, see bug 211634
    edit.clear();
    sig.insertIntoTextEdit(Signature::Start, Signature::AddSeparator | Signature::AddNewLines, &edit);
    QCOMPARE(edit.textCursor().position(), 0);
}

// Make writeConfig() public, we need it
class MySignature : public Signature
{
public:
    using Signature::readConfig;
    using Signature::writeConfig;
};

void SignatureTester::testImages()
{
    KPIMTextEdit::RichTextComposer edit;
    edit.createActions(new KActionCollection(this));

    QImage img(16, 16, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::green);
    QString image1Path = QCoreApplication::applicationDirPath() + QLatin1String("/image.png");
    img.save(image1Path);

    QImage image1, image2;
    QVERIFY(image1.load(image1Path));
    QVERIFY(image2.load(image1Path));
    QString path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/emailidentities/unittest/");
    QDir().mkpath(path);
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QStringLiteral("/signaturetest");
    QDir().mkpath(configPath);
    KConfig config(configPath);
    KConfigGroup group1 = config.group("Signature1");

    MySignature sig;
    sig.setEnabledSignature(true);
    sig.setImageLocation(path);
    sig.setInlinedHtml(true);
    sig.setText(QStringLiteral("Bla<img src=\"folder-new.png\">Bla"));
    sig.addImage(image1, QStringLiteral("folder-new.png"));
    sig.writeConfig(group1);

    // OK, signature saved, the image should be saved as well
    QDir dir(path);
    QStringList entryList = dir.entryList(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    QCOMPARE(entryList.count(), 1);
    QCOMPARE(entryList.first(), QStringLiteral("folder-new.png"));

    // Now, set the text of the signature to something without images, then save it.
    // The signature class should have removed the images.
    sig.setText(QStringLiteral("ascii ribbon campaign - against html mail"));
    sig.writeConfig(group1);
    entryList = dir.entryList(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    QCOMPARE(entryList.count(), 0);

    // Enable images again, this time with two of the buggers
    sig.setText(QStringLiteral("Bla<img src=\"folder-new.png\">Bla<img src=\"arrow-up.png\">Bla"));
    sig.addImage(image1, QStringLiteral("folder-new.png"));
    sig.addImage(image2, QStringLiteral("arrow-up.png"));
    sig.writeConfig(group1);
    entryList = dir.entryList(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    QCOMPARE(entryList.count(), 2);
    QCOMPARE(entryList.first(), QStringLiteral("arrow-up.png"));
    QCOMPARE(entryList.last(), QStringLiteral("folder-new.png"));

    // Now, create a second signature instance from the same config, and make sure it can still
    // read the images, and it does not mess up
    MySignature sig2;
    sig2.readConfig(group1);
    sig2.insertIntoTextEdit(KIdentityManagement::Signature::End, Signature::AddSeparator | Signature::AddNewLines, &edit);
    QCOMPARE(edit.composerControler()->composerImages()->embeddedImages().count(), 2);
    QCOMPARE(sig2.text(), QStringLiteral("Bla<img src=\"folder-new.png\">Bla<img src=\"arrow-up.png\">Bla"));
    sig2.writeConfig(group1);
    entryList = dir.entryList(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    QCOMPARE(entryList.count(), 2);
    QCOMPARE(entryList.first(), QStringLiteral("arrow-up.png"));
    QCOMPARE(entryList.last(), QStringLiteral("folder-new.png"));

    // Remove one image from the signature, and make sure only 1 file is left one file system.
    sig2.setText(QStringLiteral("<img src=\"folder-new.png\">"));
    sig2.writeConfig(group1);
    edit.clear();
    sig2.insertIntoTextEdit(Signature::End, Signature::AddSeparator | Signature::AddNewLines, &edit);
    QCOMPARE(edit.composerControler()->composerImages()->embeddedImages().size(), 1);
    entryList = dir.entryList(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    QCOMPARE(entryList.count(), 1);
}

void SignatureTester::testLinebreaks()
{
    Signature sig;
    sig.setEnabledSignature(true);
    sig.setType(Signature::Inlined);
    sig.setInlinedHtml(true);
    sig.setText(QStringLiteral("Hans Mustermann<br>Musterstr. 42"));

    KPIMTextEdit::RichTextComposer edit;
    edit.createActions(new KActionCollection(this));
    sig.insertIntoTextEdit(Signature::Start, Signature::AddNothing, &edit);
    QCOMPARE(edit.toPlainText(), QStringLiteral("Hans Mustermann\nMusterstr. 42"));

    edit.clear();
    sig.setText(QStringLiteral("<p>Hans Mustermann</p><br>Musterstr. 42"));
    sig.insertIntoTextEdit(Signature::Start, Signature::AddSeparator, &edit);
    QEXPECT_FAIL("", "This test is probably bogus, since Qt doesn't seem to produce HTML like this anymore.", Continue);
    QCOMPARE(edit.toPlainText(), QStringLiteral("-- \nHans Mustermann\nMusterstr. 42"));
}
