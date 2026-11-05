// SPDX-FileCopyrightText: 2026 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "../identityeditorbackend.h"

#include <QSignalSpy>
#include <QStandardPaths>
#include <QTest>

using namespace KIdentityManagementCore;
using namespace KIdentityManagementQuick;
using namespace Qt::Literals::StringLiterals;

class IdentityEditorBackendTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
    }

    void editSignature()
    {
        IdentityEditorBackend backend;
        QSignalSpy spy(&backend, &IdentityEditorBackend::identityChanged);
        QVERIFY(!backend.signatureEnabled());
        backend.setSignatureEnabled(true);
        backend.setSignatureText(u"Regards,\nAlice"_s);
        QCOMPARE(spy.count(), 2);
        QCOMPARE(backend.identity().signature().type(), Signature::Inlined);
        QVERIFY(!backend.identity().signature().isInlinedHtml());
        QCOMPARE(backend.signatureText(), u"Regards,\nAlice"_s);

        backend.setSignatureText(backend.signatureText());
        backend.setSignatureEnabled(true);
        QCOMPARE(spy.count(), 2);

        backend.setSignatureEnabled(false);
        QVERIFY(!backend.signatureEnabled());
        QCOMPARE(backend.signatureText(), u"Regards,\nAlice"_s);
        backend.setSignatureEnabled(true);
        QCOMPARE(backend.identity().signature().withSeparator(), u"-- \nRegards,\nAlice"_s);
    }

    void preserveExistingSignature()
    {
        Identity identity;
        Signature signature(u"<p>Regards,<br>Alice</p>"_s);
        signature.setInlinedHtml(true);
        signature.setEnabledSignature(true);
        identity.setSignature(signature);

        IdentityEditorBackend backend;
        backend.setIdentity(identity);
        QCOMPARE(backend.signatureText(), u"Regards,\nAlice"_s);
        QVERIFY(backend.identity().signature().isInlinedHtml());
        backend.setSignatureText(u"Plain signature"_s);
        QVERIFY(!backend.identity().signature().isInlinedHtml());
        QCOMPARE(backend.identity().signature().text(), u"Plain signature"_s);
        // Editing the backend must not change the original identity.
        QVERIFY(identity.signature().isInlinedHtml());
        QCOMPARE(identity.signature().text(), signature.text());
    }

    void replaceFileSignature()
    {
        Identity identity;
        Signature signature(u"/does/not/exist"_s, false);
        signature.setEnabledSignature(true);
        identity.setSignature(signature);
        IdentityEditorBackend backend;
        backend.setIdentity(identity);
        QVERIFY(backend.signatureText().isEmpty());
        QCOMPARE(backend.identity().signature().type(), Signature::FromFile);
        backend.setSignatureText(u"New inline signature"_s);
        QCOMPARE(backend.identity().signature().type(), Signature::Inlined);
        QCOMPARE(backend.signatureText(), u"New inline signature"_s);
    }

    void saveSignature()
    {
        auto manager = IdentityManager::self();
        const auto identity = manager->newFromScratch(u"Signature editor test"_s);
        IdentityEditorBackend backend;
        backend.setIdentity(identity);
        backend.setSignatureEnabled(true);
        backend.setSignatureText(u"Saved signature"_s);
        backend.saveIdentity(backend.identity());
        manager->rollback();

        IdentityEditorBackend reloaded;
        reloaded.setIdentityUoid(identity.uoid());
        QVERIFY(reloaded.signatureEnabled());
        QCOMPARE(reloaded.signatureText(), u"Saved signature"_s);
        QVERIFY(manager->removeIdentity(identity.identityName()));
        manager->commit();
    }
};

QTEST_GUILESS_MAIN(IdentityEditorBackendTest)
#include "identityeditorbackendtest.moc"
