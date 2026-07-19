// SPDX-FileCopyrightText: 2026 Florian Richer <florian.richer@protonmail.com>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QObject>
#include <QPointer>

#include "kidentitymanagementquick_export.h"

#include <KIdentityManagementCore/Signature>

namespace KIdentityManagementQuick
{
class IdentityEditorBackend;

/*!
 * \class KIdentityManagementQuick::SignatureEditorBackend
 * \inmodule KIdentityManagementQuick
 * \inheaderfile KIdentityManagementQuick/SignatureEditorBackend
 *
 * \brief The SignatureEditorBackend class.
 *
 * Used by the QML interface to access signature-related data.
 */
class KIDENTITYMANAGEMENTQUICK_EXPORT SignatureEditorBackend : public QObject
{
    Q_OBJECT

    Q_PROPERTY(KIdentityManagementQuick::IdentityEditorBackend *backend READ backend WRITE setBackend NOTIFY backendChanged)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(Type type READ type WRITE setType NOTIFY typeChanged)
    Q_PROPERTY(bool htmlFormat READ htmlFormat WRITE setHtmlFormat NOTIFY htmlFormatChanged)
    Q_PROPERTY(QString signatureText READ signatureText WRITE setSignatureText NOTIFY signatureTextChanged)
    Q_PROPERTY(QString file READ path WRITE setFile NOTIFY pathChanged)
    Q_PROPERTY(QString command READ path WRITE setCommand NOTIFY pathChanged)

public:
    explicit SignatureEditorBackend(QObject *parent = nullptr);

    enum Type {
        Inlined = KIdentityManagementCore::Signature::Inlined,
        FromFile = KIdentityManagementCore::Signature::FromFile,
        FromCommand = KIdentityManagementCore::Signature::FromCommand
    };
    Q_ENUM(Type)

    [[nodiscard]] IdentityEditorBackend *backend() const;
    void setBackend(IdentityEditorBackend *backend);

    [[nodiscard]] bool enabled() const;
    void setEnabled(bool enabled);

    [[nodiscard]] Type type() const;
    void setType(Type type);

    [[nodiscard]] bool htmlFormat() const;
    void setHtmlFormat(bool htmlFormat);

    [[nodiscard]] QString signatureText() const;
    void setSignatureText(const QString &signatureText);

    [[nodiscard]] QString path() const;
    void setFile(const QString &file);
    void setCommand(const QString &command);

Q_SIGNALS:
    void backendChanged();
    void enabledChanged();
    void typeChanged();
    void htmlFormatChanged();
    void signatureTextChanged();
    void pathChanged();

private:
    QPointer<IdentityEditorBackend> mBackend;

    [[nodiscard]] KIdentityManagementCore::Signature signature() const;
    void setSignature(const KIdentityManagementCore::Signature &sig);
    Type typeFromSignatureType(const KIdentityManagementCore::Signature::Type &type) const;
};
}
