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
 * \brief User identity information.
 */
class KIDENTITYMANAGEMENTCORE_EXPORT Identity
{
    Q_GADGET

    /*!
     * \property KIdentityManagementCore::Identity::mailingAllowed
     * \brief whether there are enough values set to allow mailing.
     */
    Q_PROPERTY(bool mailingAllowed READ mailingAllowed)
    /*!
     * \property KIdentityManagementCore::Identity::identityName
     * \brief the identity/nickname for this collection.
     */
    Q_PROPERTY(QString identityName READ identityName WRITE setIdentityName)
    /*!
     * \property KIdentityManagementCore::Identity::fullName
     * \brief the full name of the user.
     */
    Q_PROPERTY(QString fullName READ fullName WRITE setFullName)
    /*!
     * \property KIdentityManagementCore::Identity::organization
     * \brief the user's organization (optional).
     */
    Q_PROPERTY(QString organization READ organization WRITE setOrganization)
    /*!
     * \property KIdentityManagementCore::Identity::pgpEncryptionKey
     * \brief the user's OpenPGP encryption key.
     */
    Q_PROPERTY(QByteArray pgpEncryptionKey READ pgpEncryptionKey WRITE setPGPEncryptionKey)
    /*!
     * \property KIdentityManagementCore::Identity::pgpSigningKey
     * \brief the user's OpenPGP signing key.
     */
    Q_PROPERTY(QByteArray pgpSigningKey READ pgpSigningKey WRITE setPGPSigningKey)
    /*!
     * \property KIdentityManagementCore::Identity::smimeEncryptionKey
     * \brief the user's S/MIME encryption key.
     */
    Q_PROPERTY(QByteArray smimeEncryptionKey READ smimeEncryptionKey WRITE setSMIMEEncryptionKey)
    /*!
     * \property KIdentityManagementCore::Identity::smimeSigningKey
     * \brief the user's S/MIME signing key.
     */
    Q_PROPERTY(QByteArray smimeSigningKey READ smimeSigningKey WRITE setSMIMESigningKey)
    /*!
     * \property KIdentityManagementCore::Identity::preferredCryptoMessageFormat
     * \brief the preferred crypto message format for this identity.
     */
    Q_PROPERTY(QString preferredCryptoMessageFormat READ preferredCryptoMessageFormat WRITE setPreferredCryptoMessageFormat)
    /*!
     * \property KIdentityManagementCore::Identity::primaryEmailAddress
     * \brief the primary email address (without the user name - only name\@host).
     */
    Q_PROPERTY(QString primaryEmailAddress READ primaryEmailAddress WRITE setPrimaryEmailAddress)
    /*!
     * \property KIdentityManagementCore::Identity::emailAliases
     * \brief the email address aliases.
     */
    Q_PROPERTY(QStringList emailAliases READ emailAliases WRITE setEmailAliases)
    /*!
     * \property KIdentityManagementCore::Identity::vCardFile
     * \brief the vCard to attach to outgoing emails.
     */
    Q_PROPERTY(QString vCardFile READ vCardFile WRITE setVCardFile)
    /*!
     * \property KIdentityManagementCore::Identity::fullEmailAddr
     * \brief the email address in the format "username <name@host>".
     */
    Q_PROPERTY(QString fullEmailAddr READ fullEmailAddr)
    /*!
     * \property KIdentityManagementCore::Identity::replyToAddr
     * \brief the email address for the ReplyTo: field.
     */
    Q_PROPERTY(QString replyToAddr READ replyToAddr WRITE setReplyToAddr)
    /*!
     * \property KIdentityManagementCore::Identity::bcc
     * \brief the email addresses for the BCC: field.
     */
    Q_PROPERTY(QString bcc READ bcc WRITE setBcc)
    /*!
     * \property KIdentityManagementCore::Identity::cc
     * \brief the email addresses for the CC: field.
     */
    Q_PROPERTY(QString cc READ cc WRITE setCc)
    /*!
     * \property KIdentityManagementCore::Identity::attachVcard
     * \brief whether the vCard of this identity should be attached to outgoing mail.
     */
    Q_PROPERTY(bool attachVcard READ attachVcard WRITE setAttachVcard)
    /*!
     * \property KIdentityManagementCore::Identity::autocorrectionLanguage
     * \brief the default language for spell checking of this identity.
     */
    Q_PROPERTY(QString autocorrectionLanguage READ autocorrectionLanguage WRITE setAutocorrectionLanguage)
    /*!
     * \property KIdentityManagementCore::Identity::disabledFcc
     * \brief whether Fcc is disabled for this identity.
     */
    Q_PROPERTY(bool disabledFcc READ disabledFcc WRITE setDisabledFcc)
    /*!
     * \property KIdentityManagementCore::Identity::pgpAutoSign
     * \brief whether messages sent by this identity are signed by default.
     */
    Q_PROPERTY(bool pgpAutoSign READ pgpAutoSign WRITE setPgpAutoSign)
    /*!
     * \property KIdentityManagementCore::Identity::pgpAutoEncrypt
     * \brief whether messages sent by this identity are encrypted by default.
     */
    Q_PROPERTY(bool pgpAutoEncrypt READ pgpAutoEncrypt WRITE setPgpAutoEncrypt)
    /*!
     * \property KIdentityManagementCore::Identity::autocryptEnabled
     * \brief whether Autocrypt is enabled for this identity.
     */
    Q_PROPERTY(bool autocryptEnabled READ autocryptEnabled WRITE setAutocryptEnabled)
    /*!
     * \property KIdentityManagementCore::Identity::autocryptPrefer
     * \brief whether Autocrypt is preferred for this identity.
     */
    Q_PROPERTY(bool autocryptPrefer READ autocryptPrefer WRITE setAutocryptPrefer)
    /*!
     * \property KIdentityManagementCore::Identity::encryptionOverride
     * \brief whether the warnNotSign and warnNotEncrypt settings override the global configuration.
     */
    Q_PROPERTY(bool encryptionOverride READ encryptionOverride WRITE setEncryptionOverride)
    /*!
     * \property KIdentityManagementCore::Identity::warnNotSign
     * \brief whether to warn if parts of the message are not signed.
     */
    Q_PROPERTY(bool warnNotSign READ warnNotSign WRITE setWarnNotSign)
    /*!
     * \property KIdentityManagementCore::Identity::warnNotEncrypt
     * \brief whether to warn if parts of the message are not encrypted.
     */
    Q_PROPERTY(bool warnNotEncrypt READ warnNotEncrypt WRITE setWarnNotEncrypt)
    /*!
     * \property KIdentityManagementCore::Identity::defaultDomainName
     * \brief the default domain name.
     */
    Q_PROPERTY(QString defaultDomainName READ defaultDomainName WRITE setDefaultDomainName)
    /*!
     * \property KIdentityManagementCore::Identity::signature
     * \brief the signature of this identity.
     */
    Q_PROPERTY(Signature signature READ signature WRITE setSignature)
    /*!
     * \property KIdentityManagementCore::Identity::signatureText
     * \brief the text of the signature of this identity.
     */
    Q_PROPERTY(QString signatureText READ signatureText)
    /*!
     * \property KIdentityManagementCore::Identity::signatureIsInlinedHtml
     * \brief whether the inlined signature is HTML formatted.
     */
    Q_PROPERTY(bool signatureIsInlinedHtml READ signatureIsInlinedHtml)
    /*!
     * \property KIdentityManagementCore::Identity::transport
     * \brief the transport that is set for this identity.
     */
    Q_PROPERTY(QString transport READ transport WRITE setTransport)
    /*!
     * \property KIdentityManagementCore::Identity::fcc
     * \brief the folder where sent messages from this identity will be stored by default.
     */
    Q_PROPERTY(QString fcc READ fcc WRITE setFcc)
    /*!
     * \property KIdentityManagementCore::Identity::drafts
     * \brief the folder where draft messages from this identity will be stored by default.
     */
    Q_PROPERTY(QString drafts READ drafts WRITE setDrafts)
    /*!
     * \property KIdentityManagementCore::Identity::templates
     * \brief the folder where template messages from this identity will be stored by default.
     */
    Q_PROPERTY(QString templates READ templates WRITE setTemplates)
    /*!
     * \property KIdentityManagementCore::Identity::dictionary
     * \brief the dictionary which should be used for spell checking.
     */
    Q_PROPERTY(QString dictionary READ dictionary WRITE setDictionary)
    /*!
     * \property KIdentityManagementCore::Identity::xface
     * \brief the X-Face header for this identity.
     */
    Q_PROPERTY(QString xface READ xface WRITE setXFace)
    /*!
     * \property KIdentityManagementCore::Identity::isXFaceEnabled
     * \brief whether the X-Face header is enabled for this identity.
     */
    Q_PROPERTY(bool isXFaceEnabled READ isXFaceEnabled WRITE setXFaceEnabled)
    /*!
     * \property KIdentityManagementCore::Identity::face
     * \brief the Face header for this identity.
     */
    Q_PROPERTY(QString face READ face WRITE setFace)
    /*!
     * \property KIdentityManagementCore::Identity::isFaceEnabled
     * \brief whether the Face header is enabled for this identity.
     */
    Q_PROPERTY(bool isFaceEnabled READ isFaceEnabled WRITE setFaceEnabled)
    /*!
     * \property KIdentityManagementCore::Identity::uoid
     * \brief the Unique Object Identifier for this identity.
     */
    Q_PROPERTY(uint uoid READ uoid CONSTANT)
    /*!
     * \property KIdentityManagementCore::Identity::isNull
     * \brief whether the identity contains no values, all null values or only empty values.
     */
    Q_PROPERTY(bool isNull READ isNull)
    /*!
     * \property KIdentityManagementCore::Identity::spam
     * \brief the folder where spam messages from this identity will be stored by default.
     */
    Q_PROPERTY(QString spam READ spam WRITE setSpam)
    /*!
     * \property KIdentityManagementCore::Identity::disabledSpam
     * \brief whether Spam is disabled for this identity.
     */
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

    /*!
     * Constructs an identity named \a id with the full name \a realName,
     * the email address \a emailAddr, the organization \a organization
     * and the Reply-To address \a replyToAddress.
     */
    explicit Identity(const QString &id = QString(),
                      const QString &realName = QString(),
                      const QString &emailAddr = QString(),
                      const QString &organization = QString(),
                      const QString &replyToAddress = QString());

    /*! Returns \c true if this identity is equal to \a other. */
    bool operator==(const Identity &other) const;

    /*! Returns \c true if this identity is not equal to \a other. */
    bool operator!=(const Identity &other) const;

    /*! Returns \c true if this identity is less than \a other. Used for sorting. */
    bool operator<(const Identity &other) const;

    /*! Returns \c true if this identity is greater than \a other. Used for sorting. */
    bool operator>(const Identity &other) const;

    /*! Returns \c true if this identity is less than or equal to \a other. Used for sorting. */
    bool operator<=(const Identity &other) const;

    /*! Returns \c true if this identity is greater than or equal to \a other. Used for sorting. */
    bool operator>=(const Identity &other) const;

    /*! Returns \c true if there are enough values set to allow mailing. */
    [[nodiscard]] bool mailingAllowed() const;

    /*! Identity/nickname for this collection */
    [[nodiscard]] QString identityName() const;

    /*! Sets the identity/nickname for this collection to \a name. */
    void setIdentityName(const QString &name);

    /*! Returns whether this identity is the default identity */
    [[nodiscard]] bool isDefault() const;

    /*! Unique Object Identifier for this identity */
    [[nodiscard]] uint uoid() const;

    /*! Full name of the user */
    [[nodiscard]] QString fullName() const;
    /*! Sets the full name of the user */
    void setFullName(const QString &);

    /*! The user's organization (optional) */
    [[nodiscard]] QString organization() const;
    /*! Sets the user's organization */
    void setOrganization(const QString &);

    /*! The user's OpenPGP encryption key */
    [[nodiscard]] QByteArray pgpEncryptionKey() const;
    /*! Sets the user's OpenPGP encryption \a key. */
    void setPGPEncryptionKey(const QByteArray &key);

    /*! The user's OpenPGP signing key */
    [[nodiscard]] QByteArray pgpSigningKey() const;
    /*! Sets the user's OpenPGP signing \a key. */
    void setPGPSigningKey(const QByteArray &key);

    /*! The user's S/MIME encryption key */
    [[nodiscard]] QByteArray smimeEncryptionKey() const;
    /*! Sets the user's S/MIME encryption \a key. */
    void setSMIMEEncryptionKey(const QByteArray &key);

    /*! The user's S/MIME signing key */
    [[nodiscard]] QByteArray smimeSigningKey() const;
    /*! Sets the user's S/MIME signing \a key. */
    void setSMIMESigningKey(const QByteArray &key);

    /*! Returns the preferred crypto message format for this identity */
    [[nodiscard]] QString preferredCryptoMessageFormat() const;
    /*! Sets the preferred crypto message format for this identity */
    void setPreferredCryptoMessageFormat(const QString &);

    /*!
     * The primary email address (without the user name - only name\@host).
     *
     * This email address is used for all outgoing mail.
     *
     * \since 4.6
     */
    [[nodiscard]] QString primaryEmailAddress() const;
    /*! Sets the primary email address for this identity to \a email.
     *
     * \since 4.6
     */
    void setPrimaryEmailAddress(const QString &email);

    /*!
     * The email address aliases
     *
     * \since 4.6
     */
    [[nodiscard]] const QStringList emailAliases() const;
    /*! Sets the email address aliases for this identity to \a aliases.
     *
     * \since 4.6
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
    /*! Sets the vCard file to attach to outgoing emails */
    void setVCardFile(const QString &);

    /*!
     * The email address in the format "username <name@host>" suitable
     * for the "From:" field of email messages.
     */
    [[nodiscard]] QString fullEmailAddr() const;

    /*! Returns The email address for the ReplyTo: field */
    [[nodiscard]] QString replyToAddr() const;
    /*! Sets the email address for the ReplyTo: field */
    void setReplyToAddr(const QString &);

    /*! Returns The email addresses for the BCC: field */
    [[nodiscard]] QString bcc() const;
    /*! Sets the email addresses for the BCC: field */
    void setBcc(const QString &);

    /*!
     * Returns The email addresses for the CC: field
     * \since 4.9
     */
    [[nodiscard]] QString cc() const;
    /*! Sets the email addresses for the CC: field
     * \since 4.9
     */
    void setCc(const QString &);

    /*!
     * Returns true if the Vcard of this identity should be attached to outgoing mail.
     * \since 4.10
     */
    [[nodiscard]] bool attachVcard() const;
    /*! Sets whether the vCard of this identity should be attached to outgoing mail to \a attach.
     * \since 4.10
     */
    void setAttachVcard(bool attach);

    /*!
     * Returns The default language for spell checking of this identity.
     * \since 4.10
     */
    [[nodiscard]] QString autocorrectionLanguage() const;
    /*! Sets the default language for spell checking of this identity to \a language.
     * \since 4.10
     */
    void setAutocorrectionLanguage(const QString &language);

    /*!
     * Returns true if Fcc is disabled for this identity.
     * \since 4.11
     */
    [[nodiscard]] bool disabledFcc() const;
    /*! Sets whether Fcc is disabled for this identity
     * \since 4.11
     */
    void setDisabledFcc(bool);

    /*!
     * Returns true if we should sign message sent by this identity by default.
     * \since 4.12
     */
    [[nodiscard]] bool pgpAutoSign() const;
    /*! Sets whether messages sent by this identity should be signed by default
     * \since 4.12
     */
    void setPgpAutoSign(bool);

    /*!
     * Returns true if we should encrypt message sent by this identity by default.
     * \since 5.4
     */
    [[nodiscard]] bool pgpAutoEncrypt() const;
    /*! Sets whether messages sent by this identity should be encrypted by default
     * \since 5.4
     */
    void setPgpAutoEncrypt(bool);

    /*!
     * Returns true if Autocrypt is enabled for this identity.
     * \since 5.17
     */
    [[nodiscard]] bool autocryptEnabled() const;
    /*! Sets whether Autocrypt is enabled for this identity
     * \since 5.17
     */
    void setAutocryptEnabled(const bool);

    /*!
     * Returns true if Autocrypt is preferred for this identity.
     * \since 5.22
     */
    [[nodiscard]] bool autocryptPrefer() const;
    /*! Sets whether Autocrypt is preferred for this identity
     * \since 5.22
     */
    void setAutocryptPrefer(const bool);

    /*!
     * Returns true if the warnNotSign and warnNotEncrypt identity configuration should
     * overwrite the global app-wide configuration.
     * \since 5.22
     */
    [[nodiscard]] bool encryptionOverride() const;
    /*! Sets whether the warnNotSign and warnNotEncrypt identity configuration should
     * overwrite the global app-wide configuration
     * \since 5.22
     */
    void setEncryptionOverride(const bool);

    /*!
     * Returns true if we should warn if parts of the message this identity is about to send are not signed.
     * \since 5.22
     */
    [[nodiscard]] bool warnNotSign() const;
    /*! Sets whether to warn if parts of the message this identity is about to send are not signed
     * \since 5.22
     */
    void setWarnNotSign(const bool);

    /*!
     * Returns true if we should warn if parts of the message this identity is about to send are not encrypted.
     * \since 5.22
     */
    [[nodiscard]] bool warnNotEncrypt() const;
    /*! Sets whether to warn if parts of the message this identity is about to send are not encrypted
     * \since 5.22
     */
    void setWarnNotEncrypt(const bool);

    /*!
     * Returns The default domain name
     * \since 4.14
     */
    [[nodiscard]] QString defaultDomainName() const;
    /*! Sets the default domain name for this identity to \a domainName.
     * \since 4.14
     */
    void setDefaultDomainName(const QString &domainName);

    /*!
     * Returns The signature of the identity.
     *
     * \warning This method is not const.
     */
    [[nodiscard]] Signature &signature();
    /*! Sets the signature for this identity to \a sig. */
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
    /*! Sets the transport for this identity */
    void setTransport(const QString &);

    /*! The folder where sent messages from this identity will be
    stored by default. */
    [[nodiscard]] QString fcc() const;
    /*! Sets the folder where sent messages from this identity will be stored by default */
    void setFcc(const QString &);

    /*! The folder where draft messages from this identity will be
    stored by default.
    */
    [[nodiscard]] QString drafts() const;
    /*! Sets the folder where draft messages from this identity will be stored by default */
    void setDrafts(const QString &);

    /*! The folder where template messages from this identity will be
    stored by default.
    */
    [[nodiscard]] QString templates() const;
    /*! Sets the folder where template messages from this identity will be stored by default */
    void setTemplates(const QString &);

    /*!
     * Dictionary which should be used for spell checking
     *
     * Note that this is the localized language name (e.g. "British English"),
     * _not_ the language code or dictionary name!
     */
    [[nodiscard]] QString dictionary() const;
    /*! Sets the dictionary to be used for spell checking
     *
     * Should be the localized language name (e.g. "British English"), not the language code or dictionary name.
     */
    void setDictionary(const QString &);

    /*! a X-Face header for this identity */
    [[nodiscard]] QString xface() const;
    /*! Sets the X-Face header for this identity */
    void setXFace(const QString &);
    /*! Returns whether the X-Face header is enabled for this identity */
    [[nodiscard]] bool isXFaceEnabled() const;
    /*! Sets whether the X-Face header is enabled for this identity */
    void setXFaceEnabled(bool);

    /*! a Face header for this identity */
    [[nodiscard]] QString face() const;
    /*! Sets the Face header for this identity */
    void setFace(const QString &);
    /*! Returns whether the Face header is enabled for this identity */
    [[nodiscard]] bool isFaceEnabled() const;
    /*! Sets whether the Face header is enabled for this identity */
    void setFaceEnabled(bool);

    /*! Get random properties
     *  \a key the key of the property to get
     *  \return the property value
     */
    [[nodiscard]] QVariant property(const QString &key) const;
    /*! Set random properties
     *  \a key the key of the property to set
     *  \a value the value to set; when empty (for QStrings) or null, the property is deleted.
     */
    void setProperty(const QString &key, const QVariant &value);

    /*! Returns the null identity singleton */
    static const Identity &null();
    /*! Returns true when the identity contains no values, all null values or
    only empty values */
    [[nodiscard]] bool isNull() const;

    /*! Returns the MIME data type string for identity data */
    [[nodiscard]] static QString mimeDataType();
    /*! Returns true if the given MIME data can be decoded as an Identity */
    [[nodiscard]] static bool canDecode(const QMimeData *);
    /*! Populates the given MIME data with this identity's data */
    void populateMimeData(QMimeData *) const;
    /*! Creates an Identity from the given MIME data */
    static Identity fromMimeData(const QMimeData *);

    /*! Read configuration from config
     *  \a group the config group to read from (must be preset)
     *  Called from IdentityManager.
     */
    void readConfig(const KConfigGroup &group);

    /*! Write configuration to config
     *  \a group the config group to write to (must be preset)
     *  Called from IdentityManager.
     */
    void writeConfig(KConfigGroup &group) const;

    /*! Set whether this identity is the default identity
     *  \a flag true to set as default, false otherwise
     *
     *  Since this affects all other identities, too (most notably, the old default
     *  identity), only the IdentityManager can change this.
     *  You should use:
     *  \code
     *  kmkernel->identityManager()->setAsDefault( name_of_default )
     *  \endcode
     *  instead.
     */
    void setIsDefault(bool flag);

    /*! Set the uoid (Unique Object Identifier)
     *  \a aUoid the uoid to set
     */
    void setUoid(uint aUoid);

    /*! Returns the list of activities for this identity */
    [[nodiscard]] const QStringList activities() const;
    /*! Sets the list of activities for this identity to \a a. */
    void setActivities(const QStringList &a);

    /*! Returns whether activities are enabled for this identity */
    [[nodiscard]] bool enabledActivities() const;
    /*! Sets whether activities are enabled for this identity to \a a. */
    void setEnabledActivities(bool a);

    /*! The folder where spam messages from this identity will be
    stored by default. */
    [[nodiscard]] QString spam() const;
    /*! Sets the folder where spam messages from this identity will be stored by default */
    void setSpam(const QString &);

    /*!
     * Returns true if Spam is disabled for this identity.
     * \since 6.5
     */
    [[nodiscard]] bool disabledSpam() const;
    void setDisabledSpam(bool);

protected:
    /*! During migration, when it failed it can be a string instead of a qlonglong akonadi::id, this fixes it.
     *  Returns \a str converted to a valid Akonadi id.
     */
    [[nodiscard]] QString verifyAkonadiId(const QString &str) const;
    /*! Returns true if the signature is read from the output of a command */
    [[nodiscard]] bool signatureIsCommand() const;

    /*! Returns true if the signature is read from a text file */
    [[nodiscard]] bool signatureIsPlainFile() const;

    /*! Returns true if the signature was specified directly */
    [[nodiscard]] bool signatureIsInline() const;

    /*! name of the signature file (with path) */
    [[nodiscard]] QString signatureFile() const;
    /*! Sets the name of the signature file (with path) */
    void setSignatureFile(const QString &);

    /*! Returns the inline signature text */
    [[nodiscard]] QString signatureInlineText() const;
    /*! Sets the inline signature text */
    void setSignatureInlineText(const QString &);

    /*! Returns whether to use signature from file instead of inline signature */
    [[nodiscard]] bool useSignatureFile() const;

    Signature mSignature;
    bool mIsDefault = false;
    QHash<QString, QVariant> mPropertiesMap;
};
}

#ifndef UNITY_CMAKE_SUPPORT
Q_DECLARE_METATYPE(KIdentityManagementCore::Identity)
#endif
