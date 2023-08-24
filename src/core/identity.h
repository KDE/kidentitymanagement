/*
    SPDX-FileCopyrightText: 2002-2004 Marc Mutz <mutz@kde.org>
    SPDX-FileCopyrightText: 2007 Tom Albers <tomalbers@kde.nl>
    Author: Stefan Taferner <taferner@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kidentitymanagement_export.h"
#include "signature.h"

#include <QHash>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QVector>

namespace KIdentityManagement
{
class Identity;
}
class KConfigGroup;
class QDataStream;
class QMimeData;

namespace KIdentityManagement
{
static const char s_uoid[] = "uoid";
static const char s_identity[] = "Identity";
static const char s_name[] = "Name";
static const char s_organization[] = "Organization";
static const char s_pgps[] = "PGP Signing Key";
static const char s_pgpe[] = "PGP Encryption Key";
static const char s_smimes[] = "SMIME Signing Key";
static const char s_smimee[] = "SMIME Encryption Key";
static const char s_prefcrypt[] = "Preferred Crypto Message Format";
static const char s_primaryEmail[] = "Email Address";
static const char s_replyto[] = "Reply-To Address";
static const char s_bcc[] = "Bcc";
static const char s_cc[] = "Cc";
static const char s_vcard[] = "VCardFile";
static const char s_transport[] = "Transport";
static const char s_fcc[] = "Fcc";
static const char s_drafts[] = "Drafts";
static const char s_templates[] = "Templates";
static const char s_dict[] = "Dictionary";
static const char s_xface[] = "X-Face";
static const char s_xfaceenabled[] = "X-FaceEnabled";
static const char s_face[] = "Face";
static const char s_faceenabled[] = "FaceEnabled";
static const char s_signature[] = "Signature";
static const char s_emailAliases[] = "Email Aliases";
static const char s_attachVcard[] = "Attach Vcard";
static const char s_autocorrectionLanguage[] = "Autocorrection Language";
static const char s_disabledFcc[] = "Disable Fcc";
static const char s_encryptionOverride[] = "Override Encryption Defaults";
static const char s_pgpautosign[] = "Pgp Auto Sign";
static const char s_pgpautoencrypt[] = "Pgp Auto Encrypt";
static const char s_warnnotsign[] = "Warn not Sign";
static const char s_warnnotencrypt[] = "Warn not Encrypt";
static const char s_defaultDomainName[] = "Default Domain";
static const char s_autocryptEnabled[] = "Autocrypt";
static const char s_autocryptPrefer[] = "Autocrypt Prefer";

KIDENTITYMANAGEMENT_EXPORT QDataStream &operator<<(QDataStream &stream, const KIdentityManagement::Identity &ident);
KIDENTITYMANAGEMENT_EXPORT QDataStream &operator>>(QDataStream &stream, KIdentityManagement::Identity &ident);

/** User identity information */
class KIDENTITYMANAGEMENT_EXPORT Identity
{
    Q_GADGET

    Q_PROPERTY(bool mailingAllowed READ mailingAllowed)
    Q_PROPERTY(QString identityName READ identityName WRITE setIdentityName)
    Q_PROPERTY(QString fullName READ fullName WRITE setFullName)
    Q_PROPERTY(QString organization READ organization WRITE setOrganization)
    Q_PROPERTY(QByteArray pgpEncryptionKey READ pgpEncryptionKey WRITE setPGPEncryptionKey)
    Q_PROPERTY(QByteArray pgpSigningKey READ pgpSigningKey WRITE setPGPSigningKey)
    Q_PROPERTY(QByteArray smimeEncryptionKey READ smimeEncryptionKey WRITE setSMIMEEncryptionKey)
    Q_PROPERTY(QByteArray smimeSigningKey READ smimeSigningKey WRITE setSMIMESigningKey)
    Q_PROPERTY(QString preferredCryptoMessageFormat READ preferredCryptoMessageFormat WRITE setPreferredCryptoMessageFormat)
    Q_PROPERTY(QString primaryEmailAddress READ primaryEmailAddress WRITE setPrimaryEmailAddress)
    Q_PROPERTY(QStringList emailAliases READ emailAliases WRITE setEmailAliases)
    Q_PROPERTY(QString vCardFile READ vCardFile WRITE setVCardFile)
    Q_PROPERTY(QString fullEmailAddr READ fullEmailAddr)
    Q_PROPERTY(QString replyToAddr READ replyToAddr WRITE setReplyToAddr)
    Q_PROPERTY(QString bcc READ bcc WRITE setBcc)
    Q_PROPERTY(QString cc READ cc WRITE setCc)
    Q_PROPERTY(bool attachVcard READ attachVcard WRITE setAttachVcard)
    Q_PROPERTY(QString autocorrectionLanguage READ autocorrectionLanguage WRITE setAutocorrectionLanguage)
    Q_PROPERTY(bool disabledFcc READ disabledFcc WRITE setDisabledFcc)
    Q_PROPERTY(bool pgpAutoSign READ pgpAutoSign WRITE setPgpAutoSign)
    Q_PROPERTY(bool pgpAutoEncrypt READ pgpAutoEncrypt WRITE setPgpAutoEncrypt)
    Q_PROPERTY(bool autocryptEnabled READ autocryptEnabled WRITE setAutocryptEnabled)
    Q_PROPERTY(bool autocryptPrefer READ autocryptPrefer WRITE setAutocryptPrefer)
    Q_PROPERTY(bool encryptionOverride READ encryptionOverride WRITE setEncryptionOverride)
    Q_PROPERTY(bool warnNotSign READ warnNotSign WRITE setWarnNotSign)
    Q_PROPERTY(bool warnNotEncrypt READ warnNotEncrypt WRITE setWarnNotEncrypt)
    Q_PROPERTY(QString defaultDomainName READ defaultDomainName WRITE setDefaultDomainName)
    Q_PROPERTY(Signature signature READ signature WRITE setSignature)
    Q_PROPERTY(QString signatureText READ signatureText)
    Q_PROPERTY(bool signatureIsInlinedHtml READ signatureIsInlinedHtml)
    Q_PROPERTY(QString transport READ transport WRITE setTransport)
    Q_PROPERTY(QString fcc READ fcc WRITE setFcc)
    Q_PROPERTY(QString drafts READ drafts WRITE setDrafts)
    Q_PROPERTY(QString templates READ templates WRITE setTemplates)
    Q_PROPERTY(QString dictionary READ dictionary WRITE setDictionary)
    Q_PROPERTY(QString xface READ xface WRITE setXFace)
    Q_PROPERTY(bool isXFaceEnabled READ isXFaceEnabled WRITE setXFaceEnabled)
    Q_PROPERTY(QString face READ face WRITE setFace)
    Q_PROPERTY(bool isFaceEnabled READ isFaceEnabled WRITE setFaceEnabled)
    Q_PROPERTY(uint uoid READ uoid CONSTANT)
    Q_PROPERTY(bool isNull READ isNull)

    // only the identity manager should be able to construct and
    // destruct us, but then we get into problems with using
    // QValueList<Identity> and especially qHeapSort().
    friend class IdentityManager;

    friend KIDENTITYMANAGEMENT_EXPORT QDataStream &operator<<(QDataStream &stream, const KIdentityManagement::Identity &ident);
    friend KIDENTITYMANAGEMENT_EXPORT QDataStream &operator>>(QDataStream &stream, KIdentityManagement::Identity &ident);

public:
    using List = QVector<Identity>;

    /** Constructor */
    explicit Identity(const QString &id = QString(),
                      const QString &realName = QString(),
                      const QString &emailAddr = QString(),
                      const QString &organization = QString(),
                      const QString &replyToAddress = QString());

    /** used for comparison */
    bool operator==(const Identity &other) const;

    /** used for comparison */
    bool operator!=(const Identity &other) const;

    /** used for sorting */
    bool operator<(const Identity &other) const;

    /** used for sorting */
    bool operator>(const Identity &other) const;

    /** used for sorting */
    bool operator<=(const Identity &other) const;

    /** used for sorting */
    bool operator>=(const Identity &other) const;

    /** Tests if there are enough values set to allow mailing */
    Q_REQUIRED_RESULT bool mailingAllowed() const;

    /** Identity/nickname for this collection */
    Q_REQUIRED_RESULT QString identityName() const;

    /** Identity/nickname for this collection */
    void setIdentityName(const QString &name);

    /** @return whether this identity is the default identity */
    Q_REQUIRED_RESULT bool isDefault() const;

    /** Unique Object Identifier for this identity */
    Q_REQUIRED_RESULT uint uoid() const;

    /** Full name of the user */
    Q_REQUIRED_RESULT QString fullName() const;
    void setFullName(const QString &);

    /** The user's organization (optional) */
    Q_REQUIRED_RESULT QString organization() const;
    void setOrganization(const QString &);

    /** The user's OpenPGP encryption key */
    Q_REQUIRED_RESULT QByteArray pgpEncryptionKey() const;
    void setPGPEncryptionKey(const QByteArray &key);

    /** The user's OpenPGP signing key */
    Q_REQUIRED_RESULT QByteArray pgpSigningKey() const;
    void setPGPSigningKey(const QByteArray &key);

    /** The user's S/MIME encryption key */
    Q_REQUIRED_RESULT QByteArray smimeEncryptionKey() const;
    void setSMIMEEncryptionKey(const QByteArray &key);

    /** The user's S/MIME signing key */
    Q_REQUIRED_RESULT QByteArray smimeSigningKey() const;
    void setSMIMESigningKey(const QByteArray &key);

    Q_REQUIRED_RESULT QString preferredCryptoMessageFormat() const;
    void setPreferredCryptoMessageFormat(const QString &);

    /**
     * The primary email address (without the user name - only name\@host).
     *
     * This email address is used for all outgoing mail.
     *
     * @since 4.6
     */
    Q_REQUIRED_RESULT QString primaryEmailAddress() const;
    void setPrimaryEmailAddress(const QString &email);

    /**
     * The email address aliases
     *
     * @since 4.6
     */
    Q_REQUIRED_RESULT const QStringList emailAliases() const;
    void setEmailAliases(const QStringList &aliases);

    /**
     * @param addr the email address to check
     * @return true if this identity contains the email address @p addr, either as primary address
     *         or as alias
     *
     * @since 4.6
     */
    Q_REQUIRED_RESULT bool matchesEmailAddress(const QString &addr) const;

    /** vCard to attach to outgoing emails */
    Q_REQUIRED_RESULT QString vCardFile() const;
    void setVCardFile(const QString &);

    /**
     * The email address in the format "username <name@host>" suitable
     * for the "From:" field of email messages.
     */
    Q_REQUIRED_RESULT QString fullEmailAddr() const;

    /** @return The email address for the ReplyTo: field */
    Q_REQUIRED_RESULT QString replyToAddr() const;
    void setReplyToAddr(const QString &);

    /** @return The email addresses for the BCC: field */
    Q_REQUIRED_RESULT QString bcc() const;
    void setBcc(const QString &);

    /**
     * @return The email addresses for the CC: field
     * @since 4.9
     */
    Q_REQUIRED_RESULT QString cc() const;
    void setCc(const QString &);

    /**
     * @return true if the Vcard of this identity should be attached to outgoing mail.
     * @since 4.10
     */
    Q_REQUIRED_RESULT bool attachVcard() const;
    void setAttachVcard(bool attach);

    /**
     * @return The default language for spell checking of this identity.
     * @since 4.10
     */
    QString autocorrectionLanguage() const;
    void setAutocorrectionLanguage(const QString &language);

    /**
     * @return true if Fcc is disabled for this identity.
     * @since 4.11
     */
    Q_REQUIRED_RESULT bool disabledFcc() const;
    void setDisabledFcc(bool);

    /**
     * @return true if we should sign message sent by this identity by default.
     * @since 4.12
     */
    Q_REQUIRED_RESULT bool pgpAutoSign() const;
    void setPgpAutoSign(bool);

    /**
     * @return true if we should encrypt message sent by this identity by default.
     * @since 5.4
     */
    Q_REQUIRED_RESULT bool pgpAutoEncrypt() const;
    void setPgpAutoEncrypt(bool);

    /**
     * @return true if Autocrypt is enabled for this identity.
     * @since 5.17
     */
    Q_REQUIRED_RESULT bool autocryptEnabled() const;
    void setAutocryptEnabled(const bool);

    /**
     * @return true if Autocrypt is preferred for this identity.
     * @since 5.22
     */
    Q_REQUIRED_RESULT bool autocryptPrefer() const;
    void setAutocryptPrefer(const bool);

    /**
     * @return true if the warnNotSign and warnNotEncrypt identity configuration should
     * overwrite the global app-wide configuration.
     * @since 5.22
     */
    Q_REQUIRED_RESULT bool encryptionOverride() const;
    void setEncryptionOverride(const bool);

    /**
     * @return true if we should warn if parts of the message this identity is about to send are not signed.
     * @since 5.22
     */
    Q_REQUIRED_RESULT bool warnNotSign() const;
    void setWarnNotSign(const bool);

    /**
     * @return true if we should warn if parts of the message this identity is about to send are not encrypted.
     * @since 5.22
     */
    Q_REQUIRED_RESULT bool warnNotEncrypt() const;
    void setWarnNotEncrypt(const bool);

    /**
     * @return The default domain name
     * @since 4.14
     */
    Q_REQUIRED_RESULT QString defaultDomainName() const;
    void setDefaultDomainName(const QString &domainName);

    /**
     * @return The signature of the identity.
     *
     * @warning This method is not const.
     */
    Q_REQUIRED_RESULT Signature &signature();
    void setSignature(const Signature &sig);

    /**
     * @return the signature with '-- \n' prepended to it if it is not
     * present already.
     * No newline in front of or after the signature is added.
     * @param ok if a valid bool pointer, it is set to @c true or @c false depending
     * on whether the signature could successfully be obtained.
     */
    Q_REQUIRED_RESULT QString signatureText(bool *ok = nullptr) const;

    /**
     * @return true if the inlined signature is html formatted
     * @since 4.1
     */
    Q_REQUIRED_RESULT bool signatureIsInlinedHtml() const;

    /** The transport that is set for this identity. Used to link a
    transport with an identity. */
    Q_REQUIRED_RESULT QString transport() const;
    void setTransport(const QString &);

    /** The folder where sent messages from this identity will be
    stored by default. */
    Q_REQUIRED_RESULT QString fcc() const;
    void setFcc(const QString &);

    /** The folder where draft messages from this identity will be
    stored by default.
    */
    Q_REQUIRED_RESULT QString drafts() const;
    void setDrafts(const QString &);

    /** The folder where template messages from this identity will be
    stored by default.
    */
    Q_REQUIRED_RESULT QString templates() const;
    void setTemplates(const QString &);

    /**
     * Dictionary which should be used for spell checking
     *
     * Note that this is the localized language name (e.g. "British English"),
     * _not_ the language code or dictionary name!
     */
    Q_REQUIRED_RESULT QString dictionary() const;
    void setDictionary(const QString &);

    /** a X-Face header for this identity */
    Q_REQUIRED_RESULT QString xface() const;
    void setXFace(const QString &);
    Q_REQUIRED_RESULT bool isXFaceEnabled() const;
    void setXFaceEnabled(bool);

    /** a Face header for this identity */
    Q_REQUIRED_RESULT QString face() const;
    void setFace(const QString &);
    Q_REQUIRED_RESULT bool isFaceEnabled() const;
    void setFaceEnabled(bool);

    /** Get random properties
     *  @param key the key of the property to get
     */
    Q_REQUIRED_RESULT QVariant property(const QString &key) const;
    /** Set random properties, when @p value is empty (for QStrings) or null,
    the property is deleted. */
    void setProperty(const QString &key, const QVariant &value);

    static const Identity &null();
    /** Returns true when the identity contains no values, all null values or
    only empty values */
    Q_REQUIRED_RESULT bool isNull() const;

    Q_REQUIRED_RESULT static QString mimeDataType();
    Q_REQUIRED_RESULT static bool canDecode(const QMimeData *);
    void populateMimeData(QMimeData *) const;
    static Identity fromMimeData(const QMimeData *);

    /** Read configuration from config. Group must be preset (or use
        KConfigGroup). Called from IdentityManager. */
    void readConfig(const KConfigGroup &);

    /** Write configuration to config. Group must be preset (or use
        KConfigGroup). Called from IdentityManager. */
    void writeConfig(KConfigGroup &) const;

    /** Set whether this identity is the default identity. Since this
        affects all other identities, too (most notably, the old default
        identity), only the IdentityManager can change this.
        You should use
        <pre>
        kmkernel->identityManager()->setAsDefault( name_of_default )
        </pre>
        instead.  */
    void setIsDefault(bool flag);

    /**
     * Set the uiod
     * @param aUoid the uoid to set
     */
    void setUoid(uint aUoid);

protected:
    /** during migration when it failed it can be a string => not a qlonglong akonadi::id => fix it*/
    Q_REQUIRED_RESULT QString verifyAkonadiId(const QString &str) const;
    /** @return true if the signature is read from the output of a command */
    Q_REQUIRED_RESULT bool signatureIsCommand() const;

    /** @return true if the signature is read from a text file */
    Q_REQUIRED_RESULT bool signatureIsPlainFile() const;

    /** @return true if the signature was specified directly */
    Q_REQUIRED_RESULT bool signatureIsInline() const;

    /** name of the signature file (with path) */
    Q_REQUIRED_RESULT QString signatureFile() const;
    void setSignatureFile(const QString &);

    /** inline signature */
    Q_REQUIRED_RESULT QString signatureInlineText() const;
    void setSignatureInlineText(const QString &);

    /** Inline or signature from a file */
    Q_REQUIRED_RESULT bool useSignatureFile() const;

    Signature mSignature;
    bool mIsDefault = false;
    QHash<QString, QVariant> mPropertiesMap;
};
}
