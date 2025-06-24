/*
    SPDX-FileCopyrightText: 2008 Thomas McGuire <mcguire@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "identitytest.h"
using namespace Qt::Literals::StringLiterals;

#include "identity.h"
#include "identitymanager.h"
#include <QTest>

#include <KConfig>
#include <KConfigGroup>

#include <QDataStream>
#include <QMimeData>
#include <QStandardPaths>

using namespace KIdentityManagementCore;

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
        QCOMPARE(actual.activities(), expected.activities());
        QCOMPARE(actual.enabledActivities(), expected.enabledActivities());
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
    identity.setIdentityName(u"01234"_s);
    QCOMPARE(identity.identityName(), u"01234"_s);
    identity.setFullName(u"Daniel Vrátil"_s);
    QCOMPARE(identity.fullName(), u"Daniel Vrátil"_s);
    identity.setOrganization(u"KDE"_s);
    QCOMPARE(identity.organization(), u"KDE"_s);
    identity.setPGPEncryptionKey("0x0123456789ABCDEF");
    QCOMPARE(identity.pgpEncryptionKey(), QByteArray("0x0123456789ABCDEF"));
    identity.setPGPSigningKey("0xFEDCBA9876543210");
    QCOMPARE(identity.pgpSigningKey(), QByteArray("0xFEDCBA9876543210"));
    identity.setSMIMEEncryptionKey("0xABCDEF0123456789");
    QCOMPARE(identity.smimeEncryptionKey(), QByteArray("0xABCDEF0123456789"));
    identity.setSMIMESigningKey("0xFEDCBA9876543210");
    QCOMPARE(identity.smimeSigningKey(), QByteArray("0xFEDCBA9876543210"));
    identity.setPreferredCryptoMessageFormat(u"PGP"_s);
    QCOMPARE(identity.preferredCryptoMessageFormat(), u"PGP"_s);
    identity.setPrimaryEmailAddress(u"dvratil@kde.org"_s);
    const QStringList aliases = {u"dvratil1@kde.org"_s, u"example@example.org"_s};
    identity.setEmailAliases(aliases);
    QCOMPARE(identity.emailAliases(), aliases);
    QVERIFY(identity.matchesEmailAddress(u"dvratil@kde.org"_s));
    QVERIFY(identity.matchesEmailAddress(aliases[0]));
    QVERIFY(identity.matchesEmailAddress(aliases[1]));

    QCOMPARE(identity.primaryEmailAddress(), u"dvratil@kde.org"_s);
    const auto vcardFile = QStringLiteral(
        "BEGIN:VCARD\n"
        "VERSION:2.1\n"
        "N:Vrátil;Daniel;;\n"
        "END:VCARD");
    identity.setVCardFile(vcardFile);
    QCOMPARE(identity.vCardFile(), vcardFile);
    identity.setReplyToAddr(u"dvratil+reply@kde.org"_s);
    QCOMPARE(identity.replyToAddr(), u"dvratil+reply@kde.org"_s);
    identity.setBcc(u"dvratil+bcc@kde.org"_s);
    QCOMPARE(identity.bcc(), u"dvratil+bcc@kde.org"_s);
    identity.setCc(u"dvratil+cc@kde.org"_s);
    QCOMPARE(identity.cc(), u"dvratil+cc@kde.org"_s);
    identity.setAttachVcard(true);
    QCOMPARE(identity.attachVcard(), true);
    identity.setAutocorrectionLanguage(u"cs_CZ"_s);
    QCOMPARE(identity.autocorrectionLanguage(), u"cs_CZ"_s);
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

    identity.setDefaultDomainName(u"kde.org"_s);
    QCOMPARE(identity.defaultDomainName(), u"kde.org"_s);
    Signature sig;
    sig.setEnabledSignature(true);
    sig.setText(u"Regards,\nDaniel"_s);
    identity.setSignature(sig);
    QCOMPARE(identity.signature(), sig);
    identity.setTransport(u"smtp"_s);
    QCOMPARE(identity.transport(), u"smtp"_s);
    identity.setFcc(u"123"_s); // must be an Akonadi::Collection::Id
    QCOMPARE(identity.fcc(), u"123"_s);
    identity.setDrafts(u"124"_s);
    QCOMPARE(identity.drafts(), u"124"_s);
    identity.setTemplates(u"125"_s);
    QCOMPARE(identity.templates(), u"125"_s);
    identity.setDictionary(u"Čeština"_s);
    QCOMPARE(identity.dictionary(), u"Čeština"_s);
    identity.setXFaceEnabled(true);
    QVERIFY(identity.isXFaceEnabled());
    identity.setXFace(u":-P"_s);
    QCOMPARE(identity.xface(), u":-P"_s);
    identity.setFaceEnabled(true);
    QVERIFY(identity.isFaceEnabled());
    identity.setFace(u";-)"_s);
    QCOMPARE(identity.face(), u";-)"_s);

    identity.setSpam(u"123"_s); // must be an Akonadi::Collection::Id
    QCOMPARE(identity.spam(), u"123"_s);

    const QStringList activities = {u"foo1"_s, u"bla2"_s};
    identity.setActivities(activities);
    QCOMPARE(identity.activities(), activities);

    // Test copy
    {
        const Identity copy = identity;
        QCOMPARE(copy, identity);
        // Test that the operator==() actually works
        QVERIFY(compareIdentities(copy, identity));

        identity.setXFace(u":-("_s);
        QVERIFY(copy != identity);

        identity.setFace(u">:("_s);
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
    QVERIFY(manager.identityForAddress(u"thisaddressforsuredoesnotexist@kde.org"_s).isNull());
}

void IdentityTester::test_Aliases()
{
    IdentityManager manager;

    // It is picking up identities from older tests sometimes, so cleanup
    while (manager.identities().size() > 1) {
        QVERIFY(manager.removeIdentity(manager.identities().at(0)));
        manager.commit();
    }

    Identity &i1 = manager.newFromScratch(u"Test1"_s);
    i1.setPrimaryEmailAddress(u"firstname.lastname@example.com"_s);
    i1.setEmailAliases(QStringList() << u"firstname@example.com"_s << u"lastname@example.com"_s);
    QVERIFY(i1.matchesEmailAddress(u"\"Lastname, Firstname\" <firstname@example.com>"_s));
    QVERIFY(i1.matchesEmailAddress(u"\"Lastname, Firstname\" <firstname.lastname@example.com>"_s));
    QCOMPARE(i1.emailAliases().size(), 2);

    KConfig testConfig(u"test"_s);
    KConfigGroup testGroup(&testConfig, u"testGroup"_s);
    i1.writeConfig(testGroup);
    i1.readConfig(testGroup);
    QCOMPARE(i1.emailAliases().size(), 2);

    manager.commit();

    Identity &i2 = manager.newFromScratch(u"Test2"_s);
    i2.setPrimaryEmailAddress(u"test@test.de"_s);
    QVERIFY(i2.emailAliases().isEmpty());
    manager.commit();

    // Remove the first identity, which we couldn't remove above
    QVERIFY(manager.removeIdentity(manager.identities().at(0)));
    manager.commit();

    QCOMPARE(manager.allEmails().size(), 4);
    QCOMPARE(manager.identityForAddress(u"firstname@example.com"_s).identityName().toLatin1().data(), "Test1");
}

void IdentityTester::test_toMimeData()
{
    Identity identity(u"Test1"_s);
    identity.setFullName(u"name"_s);
    QMimeData mimeData;
    identity.populateMimeData(&mimeData);

    Identity identity2 = Identity::fromMimeData(&mimeData);

    QVERIFY(compareIdentities(identity, identity2));
    QCOMPARE(identity, identity2);

    QCOMPARE(identity.fullName(), identity2.fullName());
}

void IdentityTester::test_migration()
{
    Identity identity(u"Test1"_s);
    identity.setFullName(u"name"_s);
    QVERIFY(!identity.encryptionOverride());
    {
        KConfig config(u"test"_s);
        QVERIFY(config.isConfigWritable(true));
        KConfigGroup cg(&config, u"test"_s);
        identity.writeConfig(cg);
        config.sync();
    }
    { // Generate a config that triggers the migration code
        KConfig config(u"test_old"_s);
        QVERIFY(config.isConfigWritable(true));
        KConfigGroup cg(&config, u"test"_s);
        identity.writeConfig(cg);
        cg.deleteEntry(s_encryptionOverride);
        cg.deleteEntry(s_warnnotencrypt);
        cg.deleteEntry(s_warnnotsign);
        config.sync();
    }
    { // The migration is not triggered
        KConfig config(u"test"_s);
        KConfigGroup cg(&config, u"test"_s);
        Identity i2;
        i2.readConfig(cg);
        QVERIFY(compareIdentities(i2, identity));
    }
    { // The migration is triggered
        // for old config files (< v5.21.41)
        KConfig config(u"test_old"_s);
        KConfigGroup cg(&config, u"test"_s);
        Identity i2;
        i2.readConfig(cg);
        QVERIFY(i2.encryptionOverride());
        i2.setEncryptionOverride(false);
        QVERIFY(compareIdentities(i2, identity));
    }
}

#include "moc_identitytest.cpp"
