/*
    SPDX-FileCopyrightText: 2008 Thomas McGuire <mcguire@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "identitytest.h"
#include "identity.h"
#include "identitymanager.h"
#include <QTest>

#include <KConfig>
#include <KConfigGroup>

#include <QDataStream>
#include <QMimeData>
#include <QStandardPaths>

using namespace KIdentityManagement;

QTEST_GUILESS_MAIN(IdentityTester)

void IdentityTester::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

bool IdentityTester::compareIdentities(const Identity &actual, const Identity &expected)
{
    bool ok = false;
    [&]() {
        QCOMPARE(actual.uoid(), expected.uoid());
        // Don't compare isDefault - only one copy can be default, so this property
        // is not copied! It does not affect result of operator==() either.
        // QCOMPARE(actual.isDefault(), expected.isDefault());
        QCOMPARE(actual.identityName(), expected.identityName());
        QCOMPARE(actual.fullName(), expected.fullName());
        QCOMPARE(actual.organization(), expected.organization());
        QCOMPARE(actual.pgpEncryptionKey(), expected.pgpEncryptionKey());
        QCOMPARE(actual.pgpSigningKey(), expected.pgpSigningKey());
        QCOMPARE(actual.smimeEncryptionKey(), expected.smimeEncryptionKey());
        QCOMPARE(actual.smimeSigningKey(), expected.smimeSigningKey());
        QCOMPARE(actual.preferredCryptoMessageFormat(), expected.preferredCryptoMessageFormat());
        QCOMPARE(actual.autocryptEnabled(), expected.autocryptEnabled());
        QCOMPARE(actual.autocryptPrefer(), expected.autocryptPrefer());
        QCOMPARE(actual.encryptionOverride(), expected.encryptionOverride());
        QCOMPARE(actual.warnNotEncrypt(), expected.warnNotEncrypt());
        QCOMPARE(actual.warnNotSign(), expected.warnNotSign());
        QCOMPARE(actual.emailAliases(), expected.emailAliases());
        QCOMPARE(actual.primaryEmailAddress(), expected.primaryEmailAddress());
        QCOMPARE(actual.vCardFile(), expected.vCardFile());
        QCOMPARE(actual.replyToAddr(), expected.replyToAddr());
        QCOMPARE(actual.bcc(), expected.bcc());
        QCOMPARE(actual.cc(), expected.cc());
        QCOMPARE(actual.attachVcard(), expected.attachVcard());
        QCOMPARE(actual.autocorrectionLanguage(), expected.autocorrectionLanguage());
        QCOMPARE(actual.disabledFcc(), expected.disabledFcc());
        QCOMPARE(actual.pgpAutoSign(), expected.pgpAutoSign());
        QCOMPARE(actual.pgpAutoEncrypt(), expected.pgpAutoEncrypt());
        QCOMPARE(actual.defaultDomainName(), expected.defaultDomainName());
        QCOMPARE(actual.signatureText(), expected.signatureText());
        QCOMPARE(const_cast<Identity &>(actual).signature(), const_cast<Identity &>(expected).signature());
        QCOMPARE(actual.transport(), expected.transport());
        QCOMPARE(actual.fcc(), expected.fcc());
        QCOMPARE(actual.drafts(), expected.drafts());
        QCOMPARE(actual.templates(), expected.templates());
        QCOMPARE(actual.dictionary(), expected.dictionary());
        QCOMPARE(actual.isXFaceEnabled(), expected.isXFaceEnabled());
        QCOMPARE(actual.xface(), expected.xface());
        QCOMPARE(actual.isFaceEnabled(), expected.isFaceEnabled());
        QCOMPARE(actual.face(), expected.face());
        ok = true;
    }();

    return ok;
}

void IdentityTester::test_Identity()
{
    Identity identity;
    identity.setUoid(42);
    QCOMPARE(identity.uoid(), 42u);
    identity.setIsDefault(true);
    QCOMPARE(identity.isDefault(), true);
    identity.setIdentityName(QStringLiteral("01234"));
    QCOMPARE(identity.identityName(), QStringLiteral("01234"));
    identity.setFullName(QStringLiteral("Daniel Vrátil"));
    QCOMPARE(identity.fullName(), QStringLiteral("Daniel Vrátil"));
    identity.setOrganization(QStringLiteral("KDE"));
    QCOMPARE(identity.organization(), QStringLiteral("KDE"));
    identity.setPGPEncryptionKey("0x0123456789ABCDEF");
    QCOMPARE(identity.pgpEncryptionKey(), QByteArray("0x0123456789ABCDEF"));
    identity.setPGPSigningKey("0xFEDCBA9876543210");
    QCOMPARE(identity.pgpSigningKey(), QByteArray("0xFEDCBA9876543210"));
    identity.setSMIMEEncryptionKey("0xABCDEF0123456789");
    QCOMPARE(identity.smimeEncryptionKey(), QByteArray("0xABCDEF0123456789"));
    identity.setSMIMESigningKey("0xFEDCBA9876543210");
    QCOMPARE(identity.smimeSigningKey(), QByteArray("0xFEDCBA9876543210"));
    identity.setPreferredCryptoMessageFormat(QStringLiteral("PGP"));
    QCOMPARE(identity.preferredCryptoMessageFormat(), QStringLiteral("PGP"));
    identity.setPrimaryEmailAddress(QStringLiteral("dvratil@kde.org"));
    const QStringList aliases = {QStringLiteral("dvratil1@kde.org"), QStringLiteral("example@example.org")};
    identity.setEmailAliases(aliases);
    QCOMPARE(identity.emailAliases(), aliases);
    QVERIFY(identity.matchesEmailAddress(QStringLiteral("dvratil@kde.org")));
    QVERIFY(identity.matchesEmailAddress(aliases[0]));
    QVERIFY(identity.matchesEmailAddress(aliases[1]));

    QCOMPARE(identity.primaryEmailAddress(), QStringLiteral("dvratil@kde.org"));
    const auto vcardFile = QStringLiteral(
        "BEGIN:VCARD\n"
        "VERSION:2.1\n"
        "N:Vrátil;Daniel;;\n"
        "END:VCARD");
    identity.setVCardFile(vcardFile);
    QCOMPARE(identity.vCardFile(), vcardFile);
    identity.setReplyToAddr(QStringLiteral("dvratil+reply@kde.org"));
    QCOMPARE(identity.replyToAddr(), QStringLiteral("dvratil+reply@kde.org"));
    identity.setBcc(QStringLiteral("dvratil+bcc@kde.org"));
    QCOMPARE(identity.bcc(), QStringLiteral("dvratil+bcc@kde.org"));
    identity.setCc(QStringLiteral("dvratil+cc@kde.org"));
    QCOMPARE(identity.cc(), QStringLiteral("dvratil+cc@kde.org"));
    identity.setAttachVcard(true);
    QCOMPARE(identity.attachVcard(), true);
    identity.setAutocorrectionLanguage(QStringLiteral("cs_CZ"));
    QCOMPARE(identity.autocorrectionLanguage(), QStringLiteral("cs_CZ"));
    identity.setDisabledFcc(true);
    QVERIFY(identity.disabledFcc());
    identity.setPgpAutoSign(true);
    QVERIFY(identity.pgpAutoSign());
    identity.setPgpAutoEncrypt(true);
    QVERIFY(identity.pgpAutoEncrypt());
    QVERIFY(!identity.autocryptEnabled());
    identity.setAutocryptEnabled(true);
    QVERIFY(identity.autocryptEnabled());

    QVERIFY(!identity.autocryptPrefer());
    identity.setAutocryptPrefer(true);
    QVERIFY(identity.autocryptPrefer());

    QVERIFY(!identity.encryptionOverride());
    identity.setEncryptionOverride(true);
    QVERIFY(identity.encryptionOverride());

    QVERIFY(!identity.warnNotEncrypt());
    identity.setWarnNotEncrypt(true);
    QVERIFY(identity.warnNotEncrypt());

    QVERIFY(!identity.warnNotSign());
    identity.setWarnNotSign(true);
    QVERIFY(identity.warnNotSign());

    identity.setDefaultDomainName(QStringLiteral("kde.org"));
    QCOMPARE(identity.defaultDomainName(), QStringLiteral("kde.org"));
    Signature sig;
    sig.setEnabledSignature(true);
    sig.setText(QStringLiteral("Regards,\nDaniel"));
    identity.setSignature(sig);
    QCOMPARE(identity.signature(), sig);
    identity.setTransport(QStringLiteral("smtp"));
    QCOMPARE(identity.transport(), QStringLiteral("smtp"));
    identity.setFcc(QStringLiteral("123")); // must be an Akonadi::Collection::Id
    QCOMPARE(identity.fcc(), QStringLiteral("123"));
    identity.setDrafts(QStringLiteral("124"));
    QCOMPARE(identity.drafts(), QStringLiteral("124"));
    identity.setTemplates(QStringLiteral("125"));
    QCOMPARE(identity.templates(), QStringLiteral("125"));
    identity.setDictionary(QStringLiteral("Čeština"));
    QCOMPARE(identity.dictionary(), QStringLiteral("Čeština"));
    identity.setXFaceEnabled(true);
    QVERIFY(identity.isXFaceEnabled());
    identity.setXFace(QStringLiteral(":-P"));
    QCOMPARE(identity.xface(), QStringLiteral(":-P"));
    identity.setFaceEnabled(true);
    QVERIFY(identity.isFaceEnabled());
    identity.setFace(QStringLiteral(";-)"));
    QCOMPARE(identity.face(), QStringLiteral(";-)"));

    // Test copy
    {
        const Identity copy = identity;
        QCOMPARE(copy, identity);
        // Test that the operator==() actually works
        QVERIFY(compareIdentities(copy, identity));

        identity.setXFace(QStringLiteral(":-("));
        QVERIFY(copy != identity);

        identity.setFace(QStringLiteral(">:("));
        QVERIFY(copy != identity);
    }

    // Test serialization
    {
        QByteArray ba;
        QDataStream inStream(&ba, QIODevice::WriteOnly);
        inStream << identity;

        Identity copy;
        QDataStream outStream(&ba, QIODevice::ReadOnly);
        outStream >> copy;

        // We already verified that operator==() works, so this should be enough
        QVERIFY(compareIdentities(copy, identity));
    }
}

void IdentityTester::test_NullIdentity()
{
    IdentityManager manager;
    QVERIFY(manager.identityForAddress(QStringLiteral("thisaddressforsuredoesnotexist@kde.org")).isNull());
}

void IdentityTester::test_Aliases()
{
    IdentityManager manager;

    // It is picking up identities from older tests sometimes, so cleanup
    while (manager.identities().size() > 1) {
        QVERIFY(manager.removeIdentity(manager.identities().at(0)));
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
    QVERIFY(manager.removeIdentity(manager.identities().at(0)));
    manager.commit();

    QCOMPARE(manager.allEmails().size(), 4);
    QCOMPARE(manager.identityForAddress(QStringLiteral("firstname@example.com")).identityName().toLatin1().data(), "Test1");
}

void IdentityTester::test_toMimeData()
{
    Identity identity(QStringLiteral("Test1"));
    identity.setFullName(QStringLiteral("name"));
    QMimeData mimeData;
    identity.populateMimeData(&mimeData);

    Identity identity2 = Identity::fromMimeData(&mimeData);

    QVERIFY(compareIdentities(identity, identity2));
    QCOMPARE(identity, identity2);

    QCOMPARE(identity.fullName(), identity2.fullName());
}

void IdentityTester::test_migration()
{
    Identity identity(QStringLiteral("Test1"));
    identity.setFullName(QStringLiteral("name"));
    QVERIFY(!identity.encryptionOverride());
    {
        KConfig config(QStringLiteral("test"));
        QVERIFY(config.isConfigWritable(true));
        KConfigGroup cg(&config, QStringLiteral("test"));
        identity.writeConfig(cg);
        config.sync();
    }
    { // Generate a config that triggers the migration code
        KConfig config(QStringLiteral("test_old"));
        QVERIFY(config.isConfigWritable(true));
        KConfigGroup cg(&config, QStringLiteral("test"));
        identity.writeConfig(cg);
        cg.deleteEntry(s_encryptionOverride);
        cg.deleteEntry(s_warnnotencrypt);
        cg.deleteEntry(s_warnnotsign);
        config.sync();
    }
    { // The migration is not triggerd
        KConfig config(QStringLiteral("test"));
        KConfigGroup cg(&config, QStringLiteral("test"));
        Identity i2;
        i2.readConfig(cg);
        QVERIFY(compareIdentities(i2, identity));
    }
    { // The migration is triggered
        // for old config files (< v5.21.41)
        KConfig config(QStringLiteral("test_old"));
        KConfigGroup cg(&config, QStringLiteral("test"));
        Identity i2;
        i2.readConfig(cg);
        QVERIFY(i2.encryptionOverride());
        i2.setEncryptionOverride(false);
        QVERIFY(compareIdentities(i2, identity));
    }
}
