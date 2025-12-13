/*
    SPDX-FileCopyrightText: 2016-2025 Laurent Montel <montel@kde.org>
    SPDX-FileCopyrightText: 2008 Thomas McGuire <mcguire@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "signaturetest.h"
using namespace Qt::Literals::StringLiterals;

#include <QTest>

#include "../src/widgets/signaturerichtexteditor.cpp"
#include "signature.h"

#include <KActionCollection>
#include <KConfig>
#include <KConfigGroup>
#include <KShell>
#include <QDir>
#include <QStandardPaths>

#include <KPIMTextEdit/RichTextComposer>
#include <KPIMTextEdit/RichTextComposerControler>
#include <KPIMTextEdit/RichTextComposerImages>
using namespace KIdentityManagementCore;
using namespace KPIMTextEdit;

QTEST_MAIN(SignatureTester)

void SignatureTester::testEqualSignatures()
{
    Signature sig1;
    sig1.setText(u"Hello World"_s);
    sig1.setEnabledSignature(true);
    Signature sig2(sig1);
    QCOMPARE(sig1, sig2);
    QCOMPARE(sig2.text(), u"Hello World"_s);
    QCOMPARE(sig2.type(), Signature::Inlined);
    QCOMPARE(sig2.rawText(), u"Hello World"_s);
    QVERIFY(!sig2.isInlinedHtml());
    QCOMPARE(sig2.withSeparator(), u"-- \nHello World"_s);
    QVERIFY(sig2.isEnabledSignature());

    Signature sig3 = sig1;
    QCOMPARE(sig1, sig3);
    QCOMPARE(sig3.text(), u"Hello World"_s);
    QCOMPARE(sig3.type(), Signature::Inlined);
    QCOMPARE(sig3.rawText(), u"Hello World"_s);
    QVERIFY(!sig3.isInlinedHtml());
    QCOMPARE(sig3.withSeparator(), u"-- \nHello World"_s);
    QVERIFY(sig3.isEnabledSignature());
}

void SignatureTester::testSignatures()
{
    Signature sig1;
    sig1.setText(u"Hello World"_s);
    QCOMPARE(sig1.text(), u"Hello World"_s);
    QCOMPARE(sig1.type(), Signature::Inlined);
    QCOMPARE(sig1.rawText(), u"Hello World"_s);
    QVERIFY(!sig1.isInlinedHtml());
    QCOMPARE(sig1.withSeparator(), u"-- \nHello World"_s);
    QVERIFY(!sig1.isEnabledSignature());

    Signature sig2;
    sig2.setText(u"<b>Hello</b> World"_s);
    sig2.setInlinedHtml(true);
    QVERIFY(sig2.isInlinedHtml());
    QCOMPARE(sig2.type(), Signature::Inlined);
    QCOMPARE(sig2.rawText(), u"<b>Hello</b> World"_s);
    QCOMPARE(sig2.withSeparator(), u"-- <br><b>Hello</b> World"_s);
    QVERIFY(!sig2.isEnabledSignature());

    const QString dataFilePath = QStringLiteral(SIGNATURETEST_DATA_FILE);
    // Read this very file in, we use it for the tests
    QFile thisFile(dataFilePath);
    QVERIFY(thisFile.open(QIODevice::ReadOnly));
    QString fileContent = QString::fromUtf8(thisFile.readAll());

    if (!QStandardPaths::findExecutable(u"cat"_s).isEmpty()) {
        Signature sig3;
        sig3.setPath(u"cat "_s + KShell::quoteArg(dataFilePath), true);
        QCOMPARE(sig3.rawText(), fileContent);
        QVERIFY(!sig3.isInlinedHtml());
        QVERIFY(sig3.text().isEmpty());
        QCOMPARE(sig3.type(), Signature::FromCommand);
        QCOMPARE(sig3.withSeparator(), QString(u"-- \n"_s + fileContent));
        QVERIFY(!sig3.isEnabledSignature());
    }

    Signature sig4;
    sig4.setPath(dataFilePath, false);
    QCOMPARE(sig4.rawText(), fileContent);
    QVERIFY(!sig4.isInlinedHtml());
    QVERIFY(sig4.text().isEmpty());
    QCOMPARE(sig4.type(), Signature::FromFile);
    QCOMPARE(sig4.withSeparator(), QString(u"-- \n"_s + fileContent));
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
    sig.setText(u"Hello World"_s);

    // Test inserting signature at start, with separators. Make sure two new
    // lines are inserted before the signature

    edit.setPlainText(u"Bla Bla"_s);
    SignatureRichTextEditor::insertIntoTextEdit(sig, Signature::Start, Signature::AddSeparator | Signature::AddNewLines, &edit);
    QCOMPARE(edit.textMode(), KPIMTextEdit::RichTextComposer::Plain);
    QCOMPARE(edit.toPlainText(), u"\n\n-- \nHello World\nBla Bla"_s);

    // Test inserting signature at end. make sure cursor position is preserved
    edit.clear();
    edit.setPlainText(u"Bla Bla"_s);
    setCursorPos(edit, 4);
    SignatureRichTextEditor::insertIntoTextEdit(sig, Signature::End, Signature::AddSeparator | Signature::AddNewLines, &edit);
    QCOMPARE(edit.toPlainText(), u"Bla Bla\n-- \nHello World"_s);
    QCOMPARE(edit.textCursor().position(), 4);

    // test inserting a signature at cursor position. make sure the signature is inserted
    // to the beginning of the line and make sure modified state is preserved
    edit.clear();
    edit.setPlainText(u"Bla Bla"_s);
    setCursorPos(edit, 4);
    edit.document()->setModified(false);
    SignatureRichTextEditor::insertIntoTextEdit(sig, Signature::AtCursor, Signature::AddSeparator | Signature::AddNewLines, &edit);
    QCOMPARE(edit.toPlainText(), u"-- \nHello World\nBla Bla"_s);
    QCOMPARE(edit.textCursor().position(), 20);
    QVERIFY(!edit.document()->isModified());

    // make sure undo undoes everything in one go
    edit.undo();
    QCOMPARE(edit.toPlainText(), u"Bla Bla"_s);

    // test inserting signature without separator.
    // make sure cursor position and modified state is preserved.
    edit.clear();
    edit.setPlainText(u"Bla Bla"_s);
    setCursorPos(edit, 4);
    edit.document()->setModified(true);
    SignatureRichTextEditor::insertIntoTextEdit(sig, Signature::End, Signature::AddNewLines, &edit);
    QCOMPARE(edit.toPlainText(), u"Bla Bla\nHello World"_s);
    QCOMPARE(edit.textCursor().position(), 4);
    QVERIFY(edit.document()->isModified());

    sig.setText(u"Hello<br>World"_s);
    sig.setInlinedHtml(true);

    // test that html signatures turn html on and have correct line endings (<br> vs \n)
    edit.clear();
    edit.setPlainText(u"Bla Bla"_s);
    SignatureRichTextEditor::insertIntoTextEdit(sig, Signature::End, Signature::AddSeparator | Signature::AddNewLines, &edit);
    QCOMPARE(edit.textMode(), KPIMTextEdit::RichTextComposer::Rich);
    QCOMPARE(edit.toPlainText(), u"Bla Bla\n-- \nHello\nWorld"_s);
}

void SignatureTester::testBug167961()
{
    KPIMTextEdit::RichTextComposer edit;
    edit.createActions(new KActionCollection(this));
    Signature sig;
    sig.setEnabledSignature(true);
    sig.setText(u"BLA"_s);

    // Test that the cursor is still at the start when appending a sig into
    // an empty text edit
    SignatureRichTextEditor::insertIntoTextEdit(sig, Signature::End, Signature::AddSeparator | Signature::AddNewLines, &edit);
    QCOMPARE(edit.textCursor().position(), 0);

    // When prepending a sig, the cursor should also be at the start, see bug 211634
    edit.clear();
    SignatureRichTextEditor::insertIntoTextEdit(sig, Signature::Start, Signature::AddSeparator | Signature::AddNewLines, &edit);
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
    QString image1Path = QCoreApplication::applicationDirPath() + "/image.png"_L1;
    img.save(image1Path);

    QImage image1;
    QImage image2;
    QVERIFY(image1.load(image1Path));
    QVERIFY(image2.load(image1Path));
    QString path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + u"/emailidentities/unittest/"_s;
    QDir().mkpath(path);
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + u"/signaturetest"_s;
    QDir().mkpath(configPath);
    KConfig config(configPath);
    KConfigGroup group1 = config.group(u"Signature1"_s);

    MySignature sig;
    sig.setEnabledSignature(true);
    sig.setImageLocation(path);
    sig.setInlinedHtml(true);
    sig.setText(u"Bla<img src=\"folder-new.png\">Bla"_s);
    sig.addImage(image1, u"folder-new.png"_s);
    sig.writeConfig(group1);

    // OK, signature saved, the image should be saved as well
    QDir dir(path);
    QStringList entryList = dir.entryList(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    QCOMPARE(entryList.count(), 1);
    QCOMPARE(entryList.first(), u"folder-new.png"_s);

    // Now, set the text of the signature to something without images, then save it.
    // The signature class should have removed the images.
    sig.setText(u"ascii ribbon campaign - against html mail"_s);
    sig.writeConfig(group1);
    entryList = dir.entryList(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    QCOMPARE(entryList.count(), 0);

    // Enable images again, this time with two of the buggers
    sig.setText(u"Bla<img src=\"folder-new.png\">Bla<img src=\"arrow-up.png\">Bla"_s);
    sig.addImage(image1, u"folder-new.png"_s);
    sig.addImage(image2, u"arrow-up.png"_s);
    sig.writeConfig(group1);
    entryList = dir.entryList(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    QCOMPARE(entryList.count(), 2);
    QCOMPARE(entryList.first(), u"arrow-up.png"_s);
    QCOMPARE(entryList.last(), u"folder-new.png"_s);

    // Now, create a second signature instance from the same config, and make sure it can still
    // read the images, and it does not mess up
    MySignature sig2;
    sig2.readConfig(group1);
    SignatureRichTextEditor::insertIntoTextEdit(sig2, Signature::End, Signature::AddSeparator | Signature::AddNewLines, &edit);
    QCOMPARE(edit.composerControler()->composerImages()->embeddedImages().count(), 2);
    QCOMPARE(sig2.text(), u"Bla<img src=\"folder-new.png\">Bla<img src=\"arrow-up.png\">Bla"_s);
    sig2.writeConfig(group1);
    entryList = dir.entryList(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    QCOMPARE(entryList.count(), 2);
    QCOMPARE(entryList.first(), u"arrow-up.png"_s);
    QCOMPARE(entryList.last(), u"folder-new.png"_s);

    // Remove one image from the signature, and make sure only 1 file is left one file system.
    sig2.setText(u"<img src=\"folder-new.png\">"_s);
    sig2.writeConfig(group1);
    edit.clear();
    SignatureRichTextEditor::insertIntoTextEdit(sig2, Signature::End, Signature::AddSeparator | Signature::AddNewLines, &edit);
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
    sig.setText(u"Hans Mustermann<br>Musterstr. 42"_s);

    KPIMTextEdit::RichTextComposer edit;
    edit.createActions(new KActionCollection(this));
    SignatureRichTextEditor::insertIntoTextEdit(sig, Signature::Start, Signature::AddNothing, &edit);
    QCOMPARE(edit.toPlainText(), u"Hans Mustermann\nMusterstr. 42"_s);

    edit.clear();
    sig.setText(u"<p>Hans Mustermann</p><br>Musterstr. 42"_s);
    SignatureRichTextEditor::insertIntoTextEdit(sig, Signature::Start, Signature::AddSeparator, &edit);
    QEXPECT_FAIL("", "This test is probably bogus, since Qt doesn't seem to produce HTML like this anymore.", Continue);
    QCOMPARE(edit.toPlainText(), u"-- \nHans Mustermann\nMusterstr. 42"_s);
}

#include "moc_signaturetest.cpp"
