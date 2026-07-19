// SPDX-FileCopyrightText: 2026 Florian Richer <florian.richer@protonmail.com>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "signatureeditorbackend.h"
#include "identityeditorbackend.h"
#include "signature.h"

using namespace KIdentityManagementQuick;

SignatureEditorBackend::SignatureEditorBackend(QObject *parent)
    : QObject(parent)
{
}

IdentityEditorBackend *SignatureEditorBackend::backend() const
{
    return mBackend;
}

void SignatureEditorBackend::setBackend(IdentityEditorBackend *backend)
{
    if (backend == mBackend) {
        return;
    }

    mBackend = backend;
    Q_EMIT backendChanged();
    Q_EMIT enabledChanged();
    Q_EMIT typeChanged();
    Q_EMIT htmlFormatChanged();
    Q_EMIT signatureTextChanged();
    Q_EMIT pathChanged();
}

KIdentityManagementCore::Signature SignatureEditorBackend::signature() const
{
    if (!mBackend) {
        return {};
    }

    return mBackend->identity().signature();
}

void SignatureEditorBackend::setSignature(const KIdentityManagementCore::Signature &sig)
{
    if (!mBackend) {
        return;
    }

    auto identity = mBackend->identity();
    identity.setSignature(sig);
    mBackend->setIdentity(identity);
}

void SignatureEditorBackend::setType(Type type)
{
    if (type == this->type()) {
        return;
    }

    auto sig = signature();
    sig.setType(static_cast<KIdentityManagementCore::Signature::Type>(type));
    setSignature(sig);
    Q_EMIT typeChanged();
}

SignatureEditorBackend::Type SignatureEditorBackend::type() const
{
    return typeFromSignatureType(signature().type());
}

void SignatureEditorBackend::setEnabled(bool enabled)
{
    if (enabled == this->enabled()) {
        return;
    }

    auto sig = signature();
    sig.setEnabledSignature(enabled);
    setSignature(sig);
    Q_EMIT enabledChanged();
}

bool SignatureEditorBackend::enabled() const
{
    return signature().isEnabledSignature();
}

SignatureEditorBackend::Type SignatureEditorBackend::typeFromSignatureType(const KIdentityManagementCore::Signature::Type &type) const
{
    if (type == KIdentityManagementCore::Signature::Disabled) {
        return Inlined;
    }

    return static_cast<Type>(type);
}

bool SignatureEditorBackend::htmlFormat() const
{
    return signature().isInlinedHtml();
}

void SignatureEditorBackend::setHtmlFormat(bool htmlFormat)
{
    if (htmlFormat == this->htmlFormat()) {
        return;
    }

    auto sig = signature();
    sig.setInlinedHtml(htmlFormat);
    setSignature(sig);
    Q_EMIT htmlFormatChanged();
}

QString SignatureEditorBackend::signatureText() const
{
    return signature().text();
}

void SignatureEditorBackend::setSignatureText(const QString &signatureText)
{
    if (signatureText == this->signatureText()) {
        return;
    }

    auto sig = signature();
    sig.setText(signatureText);
    setSignature(sig);
    Q_EMIT signatureTextChanged();
}

QString SignatureEditorBackend::path() const
{
    return signature().path();
}

void SignatureEditorBackend::setFile(const QString &file)
{
    if (file == path()) {
        return;
    }

    auto sig = signature();
    sig.setPath(file, false);
    setSignature(sig);
    Q_EMIT pathChanged();
}

void SignatureEditorBackend::setCommand(const QString &command)
{
    if (command == path()) {
        return;
    }

    auto sig = signature();
    sig.setPath(command, true);
    setSignature(sig);
    Q_EMIT pathChanged();
}

#include "moc_signatureeditorbackend.cpp"
