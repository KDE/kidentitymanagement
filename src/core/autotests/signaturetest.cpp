/*
    SPDX-FileCopyrightText: 2016-2026 Laurent Montel <montel@kde.org>
    SPDX-FileCopyrightText: 2008 Thomas McGuire <mcguire@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "signaturetest.h"

#include "signature.h"
#include <KConfig>
#include <KConfigGroup>
#include <KShell>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>

using namespace Qt::Literals::StringLiterals;
using namespace KIdentityManagementCore;

QTEST_GUILESS_MAIN(SignatureTester)

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

class MySignature : public Signature
{
public:
    using Signature::readConfig;
    using Signature::writeConfig;
};

void SignatureTester::testImages()
{
    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());

    QImage img(16, 16, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::green);
    QString image1Path = temporaryDir.filePath(u"image.png"_s);
    img.save(image1Path);

    QImage image1;
    QImage image2;
    QVERIFY(image1.load(image1Path));
    QVERIFY(image2.load(image1Path));
    QString path = temporaryDir.filePath(u"images"_s);
    QDir().mkpath(path);
    QString configPath = temporaryDir.filePath(u"config"_s);
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
    QCOMPARE(sig2.embeddedImages().size(), 2);
    QCOMPARE(sig2.text(), u"Bla<img src=\"folder-new.png\">Bla<img src=\"arrow-up.png\">Bla"_s);
    sig2.writeConfig(group1);
    entryList = dir.entryList(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    QCOMPARE(entryList.count(), 2);
    QCOMPARE(entryList.first(), u"arrow-up.png"_s);
    QCOMPARE(entryList.last(), u"folder-new.png"_s);

    // Remove one image from the signature, and make sure only 1 file is left one file system.
    sig2.setText(u"<img src=\"folder-new.png\">"_s);
    sig2.writeConfig(group1);
    entryList = dir.entryList(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    QCOMPARE(entryList.count(), 1);
}

#include "moc_signaturetest.cpp"
