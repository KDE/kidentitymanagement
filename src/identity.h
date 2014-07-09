/*
    Copyright (c) 2002-2004 Marc Mutz <mutz@kde.org>
    Copyright (c) 2007 Tom Albers <tomalbers@kde.nl>
    Author: Stefan Taferner <taferner@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef KPIMIDENTITES_IDENTITY_H
#define KPIMIDENTITES_IDENTITY_H

#include "kpimidentities_export.h"
#include "signature.h"

//#include <kdemacros.h>

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QList>
#include <QtCore/QHash>
#include <QtCore/QVariant>

namespace KPIMIdentities
{
  class Identity;
  class Signature;
}
class KConfigGroup;
class QDataStream;
class QMimeData;

namespace KPIMIdentities
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
  static const char s_email[] = "Email Address"; // TODO: KDE5: Rename to s_primaryEmail
  static const char s_replyto[] = "Reply-To Address";
  static const char s_bcc[] = "Bcc";
  static const char s_cc[] = "Cc";
  static const char s_vcard[] = "VCardFile";
  static const char s_transport[] = "Transport";
  static const char s_fcc[] = "Fcc";
  static const char s_drafts[] = "Drafts";
  static const char s_templates[] = "Templates";
  static const char s_dict[] =  "Dictionary";
  static const char s_xface[] =  "X-Face";
  static const char s_xfaceenabled[] =  "X-FaceEnabled";
  static const char s_signature[] =  "Signature";
  static const char s_emailAliases[] = "Email Aliases";
  static const char s_attachVcard[] = "Attach Vcard";
  static const char s_autocorrectionLanguage[] = "Autocorrection Language";
  static const char s_disabledFcc[] = "Disable Fcc";
  static const char s_pgpautosign[] = "Pgp Auto Sign";
  static const char s_defaultDomainName[] = "Default Domain";

  KPIMIDENTITIES_EXPORT QDataStream &operator<<
  ( QDataStream &stream, const KPIMIdentities::Identity &ident );
  KPIMIDENTITIES_EXPORT QDataStream &operator>>
  ( QDataStream &stream, KPIMIdentities::Identity &ident );

/** User identity information */
class KPIMIDENTITIES_EXPORT Identity
{
    // only the identity manager should be able to construct and
    // destruct us, but then we get into problems with using
    // QValueList<Identity> and especially qHeapSort().
    friend class IdentityManager;

    friend KPIMIDENTITIES_EXPORT QDataStream &operator<<
    ( QDataStream &stream, const KPIMIdentities::Identity &ident );
    friend KPIMIDENTITIES_EXPORT QDataStream &operator>>
    ( QDataStream &stream, KPIMIdentities::Identity &ident );

    public:
      typedef QList<Identity> List;

      /** Constructor */
      explicit Identity( const QString &id=QString(),
                         const QString &realName=QString(),
                         const QString &emailAddr=QString(),
                         const QString &organization=QString(),
                         const QString &replyToAddress=QString() );

      /** Destructor */
      ~Identity();

      /** used for comparison */
      bool operator== ( const Identity &other ) const;

      /** used for comparison */
      bool operator!= ( const Identity &other ) const;

      /** used for sorting */
      bool operator< ( const Identity &other ) const;

      /** used for sorting */
      bool operator> ( const Identity &other ) const;

      /** used for sorting */
      bool operator<= ( const Identity &other ) const;

      /** used for sorting */
      bool operator>= ( const Identity &other ) const;

      /** Tests if there are enough values set to allow mailing */
      bool mailingAllowed() const;

      /** Identity/nickname for this collection */
      QString identityName() const;

      /** Identity/nickname for this collection */
      void setIdentityName( const QString &name );

      /** @return whether this identity is the default identity */
      bool isDefault() const;

      /** Unique Object Identifier for this identity */
      uint uoid() const;

      /** Full name of the user */
      QString fullName() const;
      void setFullName( const QString& );

      /** The user's organization (optional) */
      QString organization() const;
      void setOrganization( const QString& );

      /** The user's OpenPGP encryption key */
      QByteArray pgpEncryptionKey() const;
      void setPGPEncryptionKey( const QByteArray &key );

      /** The user's OpenPGP signing key */
      QByteArray pgpSigningKey() const;
      void setPGPSigningKey( const QByteArray &key );

      /** The user's S/MIME encryption key */
      QByteArray smimeEncryptionKey() const;
      void setSMIMEEncryptionKey( const QByteArray &key );

      /** The user's S/MIME signing key */
      QByteArray smimeSigningKey() const;
      void setSMIMESigningKey( const QByteArray &key );

      QString preferredCryptoMessageFormat() const;
      void setPreferredCryptoMessageFormat( const QString& );

      /**
       * primary email address (without the user name - only name\@host).
       * The primary email address is used for all outgoing mail.
       *
       * @since 4.6
       */
      QString primaryEmailAddress() const;
      void setPrimaryEmailAddress( const QString & email );

      /**
       * email address aliases
       *
       * @since 4.6
       */
      const QStringList emailAliases() const;
      void setEmailAliases( const QStringList & aliases );

      /**
       * @param addr the email address to check
       * @return true if this identity contains the email address @p addr, either as primary address
       *         or as alias
       *
       * @since 4.6
       */
      bool matchesEmailAddress( const QString & addr ) const;

      /** vCard to attach to outgoing emails */
      QString vCardFile() const;
      void setVCardFile( const QString& );

      /** email address in the format "username <name@host>" suitable
      for the "From:" field of email messages. */
      QString fullEmailAddr() const;

      /** email address for the ReplyTo: field */
      QString replyToAddr() const;
      void setReplyToAddr( const QString& );

      /** email addresses for the BCC: field */
      QString bcc() const;
      void setBcc( const QString& );

      /** email addresses for the CC: field 
       * @since 4.9
       */
      QString cc() const;
      void setCc( const QString& );

      /**
       *
       * @since 4.10
       */
      bool attachVcard() const;
      void setAttachVcard(bool attach);

      /**
       * @since 4.10
       */
      QString autocorrectionLanguage() const;
      void setAutocorrectionLanguage(const QString& language);

      /**
       * @since 4.11
       */
      bool disabledFcc() const;
      void setDisabledFcc(bool);

      /**
       * @since 4.12
       */
      bool pgpAutoSign() const;
      void setPgpAutoSign(bool);

      /**
       * @since 4.14
       */
      QString defaultDomainName() const;
      void setDefaultDomainName(const QString &domainName);


      void setSignature( const Signature &sig );
      Signature &signature(); /* _not_ const! */

      /**
      @return the signature with '-- \n' prepended to it if it is not
      present already.
      No newline in front of or after the signature is added.
      @param ok if a valid bool pointer, it is set to @c true or @c false depending
      on whether the signature could successfully be obtained.
      */
      QString signatureText( bool *ok = 0 ) const;

      /**
       * @since 4.1
       * @return true if the inlined signature is html formatted
       */
      bool signatureIsInlinedHtml() const;

      /** The transport that is set for this identity. Used to link a
      transport with an identity. */
      QString transport() const;
      void setTransport( const QString& );

      /** The folder where sent messages from this identity will be
      stored by default. */
      QString fcc() const;
      void setFcc( const QString& );

      /** The folder where draft messages from this identity will be
      stored by default.
      */
      QString drafts() const;
      void setDrafts( const QString& );

      /** The folder where template messages from this identity will be
      stored by default.
      */
      QString templates() const;
      void setTemplates( const QString& );

      /**
       * Dictionary which should be used for spell checking
       *
       * Note that this is the localized language name (e.g. "British English"),
       * _not_ the language code or dictionary name!
      */
      QString dictionary() const;
      void setDictionary( const QString& );

      /** a X-Face header for this identity */
      QString xface() const;
      void setXFace( const QString& );
      bool isXFaceEnabled() const;
      void setXFaceEnabled( const bool );

      /** Get random properties
       *  @param key the key of the property to get
       */
      QVariant property( const QString &key ) const;
      /** Set random properties, when @p value is empty (for QStrings) or null,
      the property is deleted. */
      void setProperty( const QString &key, const QVariant &value );

      static const Identity &null();
      /** Returns true when the identity contains no values, all null values or
      only empty values */
      bool isNull() const;

      static QString mimeDataType();
      static bool canDecode( const QMimeData* );
      void populateMimeData( QMimeData* );
      static Identity fromMimeData( const QMimeData* );

      /** Read configuration from config. Group must be preset (or use
          KConfigGroup). Called from IdentityManager. */
      void readConfig( const KConfigGroup & );

      /** Write configuration to config. Group must be preset (or use
          KConfigGroup). Called from IdentityManager. */
      void writeConfig( KConfigGroup & ) const;

      /** Set whether this identity is the default identity. Since this
          affects all other identites, too (most notably, the old default
          identity), only the IdentityManager can change this.
          You should use
          <pre>
          kmkernel->identityManager()->setAsDefault( name_of_default )
          </pre>
          instead.  */
      void setIsDefault( bool flag );

      /** set the uiod
       *  @param aUoid the uoid to set
       */
      void setUoid( uint aUoid );

    protected:
      /** during migration when it failed it can be a string => not a qlonglong akonadi::id => fix it*/
      QString verifyAkonadiId(const QString& str) const;
      /** @return true if the signature is read from the output of a command */
      bool signatureIsCommand() const;

      /** @return true if the signature is read from a text file */
      bool signatureIsPlainFile() const;

      /** @return true if the signature was specified directly */
      bool signatureIsInline() const;

      /** name of the signature file (with path) */
      QString signatureFile() const;
      void setSignatureFile( const QString& );

      /** inline signature */
      QString signatureInlineText() const;
      void setSignatureInlineText( const QString& );

      /** Inline or signature from a file */
      bool useSignatureFile() const;

      Signature mSignature;
      bool      mIsDefault;
      QHash<QString, QVariant>   mPropertiesMap;
};

}

#endif /*kpim_identity_h*/
