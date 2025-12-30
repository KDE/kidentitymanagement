/*
    SPDX-FileCopyrightText: 2002-2004 Marc Mutz <mutz@kde.org>
    SPDX-FileCopyrightText: 2007 Tom Albers <tomalbers@kde.nl>
    Author: Stefan Taferner <taferner@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kidentitymanagementcore_export.h"

#include "signature.h"

#include <QHash>
#include <QList>
#include <QString>
#include <QStringList>
#include <QVariant>

namespace KIdentityManagementCore
{
class Identity;
}
class KConfigGroup;
class QDataStream;
class QMimeData;

namespace KIdentityManagementCore
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
static const char s_activities[] = "Activities";
static const char s_enabledActivities[] = "Enabled Activities";
static const char s_spam[] = "Spam";
static const char s_disabledSpam[] = "Disable Spam";

KIDENTITYMANAGEMENTCORE_EXPORT QDataStream &operator<<(QDataStream &stream, const KIdentityManagementCore::Identity &ident);
KIDENTITYMANAGEMENTCORE_EXPORT QDataStream &operator>>(QDataStream &stream, KIdentityManagementCore::Identity &ident);

/*!
 * \class KIdentityManagementCore::Identity
 * \inmodule KIdentityManagementCore
 * \inheaderfile KIdentityManagementCore/Identity
 *
 * \brief User identity information
 */
class KIDENTITYMANAGEMENTCORE_EXPORT Identity
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
    Q_PROPERTY(QString spam READ spam WRITE setSpam)
    Q_PROPERTY(bool disabledSpam READ disabledSpam WRITE setDisabledSpam)

    // only the identity manager should be able to construct and
    // destruct us, but then we get into problems with using
    // QValueList<Identity> and especially qHeapSort().
    friend class IdentityManager;

    friend KIDENTITYMANAGEMENTCORE_EXPORT QDataStream &operator<<(QDataStream &stream, const KIdentityManagementCore::Identity &ident);
    friend KIDENTITYMANAGEMENTCORE_EXPORT QDataStream &operator>>(QDataStream &stream, KIdentityManagementCore::Identity &ident);

public:
    using List = QList<Identity>;
    using Id = uint;

    /*! Constructor */
    explicit Identity(const QString &id = QString(),
                      const QString &realName = QString(),
                      const QString &emailAddr = QString(),
                      const QString &organization = QString(),
                      const QString &replyToAddress = QString());

    /*! used for comparison */
    bool operator==(const Identity &other) const;

    /*! used for comparison */
    bool operator!=(const Identity &other) const;

    /*! used for sorting */
    bool operator<(const Identity &other) const;

    /*! used for sorting */
    bool operator>(const Identity &other) const;

    /*! used for sorting */
    bool operator<=(const Identity &other) const;

    /*! used for sorting */
    bool operator>=(const Identity &other) const;

    /*! Tests if there are enough values set to allow mailing */
    [[nodiscard]] bool mailingAllowed() const;

    /*! Identity/nickname for this collection */
    [[nodiscard]] QString identityName() const;

    /*! Identity/nickname for this collection */
    void setIdentityName(const QString &name);

    /*! Returns whether this identity is the default identity */
    [[nodiscard]] bool isDefault() const;

    /*! Unique Object Identifier for this identity */
    [[nodiscard]] uint uoid() const;

    /*! Full name of the user */
    [[nodiscard]] QString fullName() const;
    /*!
     */
    void setFullName(const QString &);

    /*! The user's organization (optional) */
    [[nodiscard]] QString organization() const;
    /*!
     */
    void setOrganization(const QString &);

    /*! The user's OpenPGP encryption key */
    [[nodiscard]] QByteArray pgpEncryptionKey() const;
    /*!
     */
    void setPGPEncryptionKey(const QByteArray &key);

    /*! The user's OpenPGP signing key */
    [[nodiscard]] QByteArray pgpSigningKey() const;
    /*!
     */
    void setPGPSigningKey(const QByteArray &key);

    /*! The user's S/MIME encryption key */
    [[nodiscard]] QByteArray smimeEncryptionKey() const;
    /*!
     */
    void setSMIMEEncryptionKey(const QByteArray &key);

    /*! The user's S/MIME signing key */
    [[nodiscard]] QByteArray smimeSigningKey() const;
    /*!
     */
    void setSMIMESigningKey(const QByteArray &key);

    /*!
     */
    [[nodiscard]] QString preferredCryptoMessageFormat() const;
    /*!
     */
    void setPreferredCryptoMessageFormat(const QString &);

    /*!
     * The primary email address (without the user name - only name\@host).
     *
     * This email address is used for all outgoing mail.
     *
     * \since 4.6
     */
    [[nodiscard]] QString primaryEmailAddress() const;
    /*!
     */
    void setPrimaryEmailAddress(const QString &email);

    /*!
     * The email address aliases
     *
     * \since 4.6
     */
    [[nodiscard]] const QStringList emailAliases() const;
    /*!
     */
    void setEmailAliases(const QStringList &aliases);

    /*!
     * \a addr the email address to check
     * Returns true if this identity contains the email address \a addr, either as primary address
     *         or as alias
     *
     * \since 4.6
     */
    [[nodiscard]] bool matchesEmailAddress(const QString &addr) const;

    /*! vCard to attach to outgoing emails */
    [[nodiscard]] QString vCardFile() const;
    void setVCardFile(const QString &);

    /*!
     * The email address in the format "username <name@host>" suitable
     * for the "From:" field of email messages.
     */
    [[nodiscard]] QString fullEmailAddr() const;

    /*! Returns The email address for the ReplyTo: field */
    [[nodiscard]] QString replyToAddr() const;
    /*!
     */
    void setReplyToAddr(const QString &);

    /*! Returns The email addresses for the BCC: field */
    [[nodiscard]] QString bcc() const;
    /*!
     */
    void setBcc(const QString &);

    /*!
     * Returns The email addresses for the CC: field
     * \since 4.9
     */
    [[nodiscard]] QString cc() const;
    /*!
     */
    void setCc(const QString &);

    /*!
     * Returns true if the Vcard of this identity should be attached to outgoing mail.
     * \since 4.10
     */
    [[nodiscard]] bool attachVcard() const;
    /*!
     */
    void setAttachVcard(bool attach);

    /*!
     * Returns The default language for spell checking of this identity.
     * \since 4.10
     */
    [[nodiscard]] QString autocorrectionLanguage() const;
    /*!
     */
    void setAutocorrectionLanguage(const QString &language);

    /*!
     * Returns true if Fcc is disabled for this identity.
     * \since 4.11
     */
    [[nodiscard]] bool disabledFcc() const;
    /*!
     */
    void setDisabledFcc(bool);

    /*!
     * Returns true if we should sign message sent by this identity by default.
     * \since 4.12
     */
    [[nodiscard]] bool pgpAutoSign() const;
    /*!
     */
    void setPgpAutoSign(bool);

    /*!
     * Returns true if we should encrypt message sent by this identity by default.
     * \since 5.4
     */
    [[nodiscard]] bool pgpAutoEncrypt() const;
    /*!
     */
    void setPgpAutoEncrypt(bool);

    /*!
     * Returns true if Autocrypt is enabled for this identity.
     * \since 5.17
     */
    [[nodiscard]] bool autocryptEnabled() const;
    /*!
     */
    void setAutocryptEnabled(const bool);

    /*!
     * Returns true if Autocrypt is preferred for this identity.
     * \since 5.22
     */
    [[nodiscard]] bool autocryptPrefer() const;
    /*!
     */
    void setAutocryptPrefer(const bool);

    /*!
     * Returns true if the warnNotSign and warnNotEncrypt identity configuration should
     * overwrite the global app-wide configuration.
     * \since 5.22
     */
    [[nodiscard]] bool encryptionOverride() const;
    /*!
     */
    void setEncryptionOverride(const bool);

    /*!
     * Returns true if we should warn if parts of the message this identity is about to send are not signed.
     * \since 5.22
     */
    [[nodiscard]] bool warnNotSign() const;
    /*!
     */
    void setWarnNotSign(const bool);

    /*!
     * Returns true if we should warn if parts of the message this identity is about to send are not encrypted.
     * \since 5.22
     */
    [[nodiscard]] bool warnNotEncrypt() const;
    /*!
     */
    void setWarnNotEncrypt(const bool);

    /*!
     * Returns The default domain name
     * \since 4.14
     */
    [[nodiscard]] QString defaultDomainName() const;
    /*!
     */
    void setDefaultDomainName(const QString &domainName);

    /*!
     * Returns The signature of the identity.
     *
     * \warning This method is not const.
     */
    [[nodiscard]] Signature &signature();
    /*!
     */
    void setSignature(const Signature &sig);

    /*!
     * Returns the signature with '-- \n' prepended to it if it is not
     * present already.
     * No newline in front of or after the signature is added.
     * \a ok if a valid bool pointer, it is set to \\ true or \\ false depending
     * on whether the signature could successfully be obtained.
     */
    [[nodiscard]] QString signatureText(bool *ok = nullptr) const;

    /*!
     * Returns true if the inlined signature is html formatted
     * \since 4.1
     */
    [[nodiscard]] bool signatureIsInlinedHtml() const;

    /*! The transport that is set for this identity. Used to link a
    transport with an identity. */
    [[nodiscard]] QString transport() const;
    /*!
     */
    void setTransport(const QString &);

    /*! The folder where sent messages from this identity will be
    stored by default. */
    [[nodiscard]] QString fcc() const;
    /*!
     */
    void setFcc(const QString &);

    /*! The folder where draft messages from this identity will be
    stored by default.
    */
    [[nodiscard]] QString drafts() const;
    /*!
     */
    void setDrafts(const QString &);

    /*! The folder where template messages from this identity will be
    stored by default.
    */
    [[nodiscard]] QString templates() const;
    /*!
     */
    void setTemplates(const QString &);

    /*!
     * Dictionary which should be used for spell checking
     *
     * Note that this is the localized language name (e.g. "British English"),
     * _not_ the language code or dictionary name!
     */
    [[nodiscard]] QString dictionary() const;
    /*!
     */
    void setDictionary(const QString &);

    /*! a X-Face header for this identity */
    [[nodiscard]] QString xface() const;
    /*!
     */
    void setXFace(const QString &);
    /*!
     */
    [[nodiscard]] bool isXFaceEnabled() const;
    /*!
     */
    void setXFaceEnabled(bool);

    /*! a Face header for this identity */
    [[nodiscard]] QString face() const;
    /*!
     */
    void setFace(const QString &);
    /*!
     */
    [[nodiscard]] bool isFaceEnabled() const;
    /*!
     */
    void setFaceEnabled(bool);

    /*! Get random properties
     *  \a key the key of the property to get
     */
    [[nodiscard]] QVariant property(const QString &key) const;
    /*! Set random properties, when \a value is empty (for QStrings) or null,
    the property is deleted. */
    void setProperty(const QString &key, const QVariant &value);

    static const Identity &null();
    /*! Returns true when the identity contains no values, all null values or
    only empty values */
    [[nodiscard]] bool isNull() const;

    /*!
     */
    [[nodiscard]] static QString mimeDataType();
    /*!
     */
    [[nodiscard]] static bool canDecode(const QMimeData *);
    /*!
     */
    void populateMimeData(QMimeData *) const;
    /*!
     */
    static Identity fromMimeData(const QMimeData *);

    /*! Read configuration from config. Group must be preset (or use
        KConfigGroup). Called from IdentityManager. */
    void readConfig(const KConfigGroup &);

    /*! Write configuration to config. Group must be preset (or use
        KConfigGroup). Called from IdentityManager. */
    void writeConfig(KConfigGroup &) const;

    /*! Set whether this identity is the default identity. Since this
        affects all other identities, too (most notably, the old default
        identity), only the IdentityManager can change this.
        You should use
        <pre>
        kmkernel->identityManager()->setAsDefault( name_of_default )
        </pre>
        instead.  */
    void setIsDefault(bool flag);

    /*!
     * Set the uiod
     * \a aUoid the uoid to set
     */
    void setUoid(uint aUoid);

    /*!
     */
    [[nodiscard]] const QStringList activities() const;
    /*!
     */
    void setActivities(const QStringList &a);

    /*!
     */
    [[nodiscard]] bool enabledActivities() const;
    /*!
     */
    void setEnabledActivities(bool a);

    /*! The folder where spam messages from this identity will be
    stored by default. */
    [[nodiscard]] QString spam() const;
    /*!
     */
    void setSpam(const QString &);

    /*!
     * Returns true if Spam is disabled for this identity.
     * \since 6.5
     */
    [[nodiscard]] bool disabledSpam() const;
    void setDisabledSpam(bool);

protected:
    /*! during migration when it failed it can be a string => not a qlonglong akonadi::id => fix it*/
    [[nodiscard]] QString verifyAkonadiId(const QString &str) const;
    /*! Returns true if the signature is read from the output of a command */
    [[nodiscard]] bool signatureIsCommand() const;

    /*! Returns true if the signature is read from a text file */
    [[nodiscard]] bool signatureIsPlainFile() const;

    /*! Returns true if the signature was specified directly */
    [[nodiscard]] bool signatureIsInline() const;

    /*! name of the signature file (with path) */
    [[nodiscard]] QString signatureFile() const;
    /*!
     */
    void setSignatureFile(const QString &);

    /*! inline signature */
    [[nodiscard]] QString signatureInlineText() const;
    /*!
     */
    void setSignatureInlineText(const QString &);

    /*! Inline or signature from a file */
    [[nodiscard]] bool useSignatureFile() const;

    Signature mSignature;
    bool mIsDefault = false;
    QHash<QString, QVariant> mPropertiesMap;
};
}

#ifndef UNITY_CMAKE_SUPPORT
Q_DECLARE_METATYPE(KIdentityManagementCore::Identity)
#endif
