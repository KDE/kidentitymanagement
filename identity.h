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

#ifndef kpim_identity_h
#define kpim_identity_h

#include "kpimidentities_export.h"
#include "signature.h"

#include <kdemacros.h>

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
  static const char s_email[] = "Email Address";
  static const char s_replyto[] = "Reply-To Address";
  static const char s_bcc[] = "Bcc";
  static const char s_vcard[] = "VCardFile";
  static const char s_transport[] = "Transport";
  static const char s_fcc[] = "Fcc";
  static const char s_drafts[] = "Drafts";
  static const char s_templates[] = "Templates";
  static const char s_dict[] =  "Dictionary";
  static const char s_xface[] =  "X-Face";
  static const char s_xfaceenabled[] =  "X-FaceEnabled";
  static const char s_signature[] =  "Signature";

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

      /** email address (without the user name - only name\@host) */
      QString emailAddr() const;
      void setEmailAddr( const QString& );

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

      void setSignature( const Signature &sig );
      Signature &signature(); /* _not_ const! */

      /** Returns the signature. This method also takes care of special
      signature files that are shell scripts and handles them
      correct. So use this method to rectreive the contents of the
      signature file. If @p prompt is false, no errors will be displayed
      (useful for retries). */
      QString signatureText( bool *ok=0 ) const;

      /** The transport ID that is set for this identity. Used to link a
      transport with an identity. The IDs given here correspond to the
      transport IDs used by the mailtransport library. */
      int transport() const;
      void setTransport( int );

      /** The folder where sent messages from this identity will be
      stored by default. */
      QString fcc() const;
      void setFcc( const QString& );

      /** The folder where draft messages from this identity will be
      stored by default. */
      QString drafts() const;
      void setDrafts( const QString& );

      /** The folder where template messages from this identity will be
      stored by default. */
      QString templates() const;
      void setTemplates( const QString& );

      /** dictionary which should be used for spell checking */
      QString dictionary() const;
      void setDictionary( const QString& );

      /** a X-Face header for this identity */
      QString xface() const;
      void setXFace( const QString& );
      bool isXFaceEnabled() const;
      void setXFaceEnabled( const bool );

      /** Get random properties */
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

    protected:
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

      /** set the uiod */
      void setUoid( uint aUoid );

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
      QHash<QString,QVariant>   mPropertiesMap;
  };

}

#endif /*kpim_identity_h*/
