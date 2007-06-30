/*
    Copyright (c) 2002-2004 Marc Mutz <mutz@kde.org>
    Copyright (c) 2007 Tom Albers <tomalbers@kde.nl>

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

#include "identity.h"

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <kurl.h>
#include <kprocess.h>
#include <kpimutils/kfileio.h>

#include <QFileInfo>
#include <QMimeData>
#include <QByteArray>

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>

using namespace KPIMIdentities;

Signature::Signature()
  : mType( Disabled )
{
}

Signature::Signature( const QString & text )
  : mText( text ),
    mType( Inlined )
{
}

Signature::Signature( const QString & url, bool isExecutable )
  : mUrl( url ),
    mType( isExecutable ? FromCommand : FromFile )
{
}

bool Signature::operator==( const Signature & other ) const {
  if ( mType != other.mType ) return false;
  switch ( mType ) {
  case Inlined: return mText == other.mText;
  case FromFile:
  case FromCommand: return mUrl == other.mUrl;
  default:
  case Disabled: return true;
  }
}

QString Signature::rawText( bool * ok ) const
{
  switch ( mType ) {
  case Disabled:
    if ( ok ) *ok = true;
    return QString();
  case Inlined:
    if ( ok ) *ok = true;
    return mText;
  case FromFile:
    return textFromFile( ok );
  case FromCommand:
    return textFromCommand( ok );
  };
  kFatal( 5325 ) << "Signature::type() returned unknown value!" << endl;
  return QString(); // make compiler happy
}

QString Signature::textFromCommand( bool * ok ) const
{
  assert( mType == FromCommand );

  // handle pathological cases:
  if ( mUrl.isEmpty() ) {
    if ( ok ) *ok = true;
    return QString();
  }

  // create a shell process:
  KProcess proc;
  proc.setOutputChannelMode( KProcess::SeparateChannels );
  proc.setShellCommand( mUrl );
  int rc = proc.execute();

  // handle errors, if any:
  if ( rc != 0 ) {
    if ( ok ) *ok = false;
    QString wmsg = i18n("<qt>Failed to execute signature script<br><b>%1</b>:"
            "<br>%2</qt>", mUrl, QString( proc.readAllStandardError() ) );
    KMessageBox::error(0, wmsg);
    return QString();
  }

  // no errors:
  if ( ok )
      *ok = true;

  // get output:
  QByteArray output = proc.readAllStandardOutput();

  // TODO: hmm, should we allow other encodings, too?
  return QString::fromLocal8Bit( output.data(), output.size() );
}

QString Signature::textFromFile( bool * ok ) const
{
  assert( mType == FromFile );

  // TODO: Use KIO::NetAccess to download non-local files!
  if ( !KUrl(mUrl).isLocalFile() && !(QFileInfo(mUrl).isRelative()
                                      && QFileInfo(mUrl).exists()) ) {
      kDebug( 5325 ) << "Signature::textFromFile: "
              << "non-local URLs are unsupported" << endl;
    if ( ok )
        *ok = false;
    return QString();
  }

  if ( ok )
      *ok = true;

  // TODO: hmm, should we allow other encodings, too?
  const QByteArray ba = KPIMUtils::kFileToByteArray( mUrl, false );
  return QString::fromLocal8Bit( ba.data(), ba.size() );
}

QString Signature::withSeparator( bool * ok ) const
{
  bool internalOK = false;
  QString signature = rawText( &internalOK );
  if ( !internalOK ) {
    if ( ok )
        *ok = false;
    return QString();
  }
  if ( ok )
      *ok = true;

  if ( signature.isEmpty() )
      return signature; // don't add a separator in this case

  if ( signature.startsWith( QString::fromLatin1("-- \n") ) )
    // already have signature separator at start of sig:
    return QString::fromLatin1("\n") += signature;
  else if ( signature.indexOf( QString::fromLatin1("\n-- \n") ) != -1 )
    // already have signature separator inside sig; don't prepend '\n'
    // to improve abusing signatures as templates:
    return signature;
  else
    // need to prepend one:
    return QString::fromLatin1("\n-- \n") + signature;
}


void Signature::setUrl( const QString & url, bool isExecutable )
{
  mUrl = url;
  mType = isExecutable ? FromCommand : FromFile ;
}

// config keys and values:
static const char sigTypeKey[] = "Signature Type";
static const char sigTypeInlineValue[] = "inline";
static const char sigTypeFileValue[] = "file";
static const char sigTypeCommandValue[] = "command";
static const char sigTypeDisabledValue[] = "disabled";
static const char sigTextKey[] = "Inline Signature";
static const char sigFileKey[] = "Signature File";
static const char sigCommandKey[] = "Signature Command";

void Signature::readConfig( const KConfigGroup &config )
{
  QString sigType = config.readEntry( sigTypeKey );
  if ( sigType == sigTypeInlineValue ) {
    mType = Inlined;
  } else if ( sigType == sigTypeFileValue ) {
    mType = FromFile;
    mUrl = config.readPathEntry( sigFileKey );
  } else if ( sigType == sigTypeCommandValue ) {
    mType = FromCommand;
    mUrl = config.readPathEntry( sigCommandKey );
  } else {
    mType = Disabled;
  }
  mText = config.readEntry( sigTextKey );
}

void Signature::writeConfig( KConfigGroup & config ) const
{
  switch ( mType ) {
  case Inlined:
    config.writeEntry( sigTypeKey, sigTypeInlineValue );
    break;
  case FromFile:
    config.writeEntry( sigTypeKey, sigTypeFileValue );
    config.writePathEntry( sigFileKey, mUrl );
    break;
  case FromCommand:
    config.writeEntry( sigTypeKey, sigTypeCommandValue );
    config.writePathEntry( sigCommandKey, mUrl );
    break;
  case Disabled:
    config.writeEntry( sigTypeKey, sigTypeDisabledValue );
  default: ;
  }
  config.writeEntry( sigTextKey, mText );
}

QDataStream & KPIMIdentities::operator<<
        ( QDataStream & stream, const KPIMIdentities::Signature & sig )
{
  return stream << static_cast<quint8>(sig.mType) << sig.mUrl << sig.mText;
}

QDataStream & KPIMIdentities::operator>>
        ( QDataStream & stream, KPIMIdentities::Signature & sig )
{
  quint8 s;
  stream >> s  >> sig.mUrl >> sig.mText;
  sig.mType = static_cast<Signature::Type>(s);
  return stream;
}

// TODO: should use a kstaticdeleter?
static Identity* identityNull = 0;
const Identity& Identity::null()
{
  if ( !identityNull )
    identityNull = new Identity;
  return *identityNull;
}

bool Identity::isNull() const
{
  bool empty = true;
  QHash<QString, QVariant>::const_iterator i = mPropertiesMap.constBegin();
  while (i != mPropertiesMap.constEnd()) {
      if ( !i.value().isNull() ||
           ( i.value().type() == QVariant::String && !i.value().toString().isEmpty() ) )
          empty = false;
      ++i;
  }
  return empty;
}

bool Identity::operator==( const Identity & other ) const
{
  return mPropertiesMap == other.mPropertiesMap;
}

Identity::Identity( const QString & id, const QString & fullName,
			        const QString & emailAddr, const QString & organization,
			        const QString & replyToAddr )
{
    setProperty(s_uoid, id);
    setProperty(s_name, fullName);
    setProperty(s_email, emailAddr);
    setProperty(s_organization, organization);
    setProperty(s_replyto, replyToAddr);
}

Identity::~Identity()
{
}


void Identity::readConfig( const KConfigGroup & config )
{
  // get all keys and convert them to our QHash.
  QMap<QString,QString> entries = config.entryMap();
  QMap<QString,QString>::const_iterator i = entries.constBegin();
  while (i != entries.constEnd()) {
    mPropertiesMap.insert(i.key(), i.value());
    ++i;
  }
}


void Identity::writeConfig( KConfigGroup & config ) const
{
  QHash<QString, QVariant>::const_iterator i = mPropertiesMap.constBegin();
  while (i != mPropertiesMap.constEnd()) {
      config.writeEntry( i.key(), i.value() );
      kDebug( 5325 ) << "Store: " << i.key() << ": " << i.value() << endl;
      ++i;
  }
  mSignature.writeConfig( config );
}

QDataStream & KPIMIdentities::operator<<
        ( QDataStream & stream, const KPIMIdentities::Identity & i )
{
  return stream << static_cast<quint32>(i.uoid())
		<< i.identityName()
		<< i.fullName()
		<< i.organization()
		<< i.pgpSigningKey()
		<< i.pgpEncryptionKey()
		<< i.smimeSigningKey()
		<< i.smimeEncryptionKey()
		<< i.emailAddr()
		<< i.replyToAddr()
		<< i.bcc()
		<< i.vCardFile()
		<< i.transport()
		<< i.fcc()
		<< i.drafts()
		<< i.templates()
        << i.mPropertiesMap[s_signature]
        << i.dictionary()
        << i.xface()
        << i.preferredCryptoMessageFormat();
}

QDataStream & KPIMIdentities::operator>>
        ( QDataStream & stream, KPIMIdentities::Identity & i )
{
  quint32 uoid;
  QString format;
  stream
        >> uoid
        >> i.mPropertiesMap[s_identity]
        >> i.mPropertiesMap[s_name]
        >> i.mPropertiesMap[s_organization]
        >> i.mPropertiesMap[s_pgps]
        >> i.mPropertiesMap[s_pgpe]
        >> i.mPropertiesMap[s_smimes]
        >> i.mPropertiesMap[s_smimee]
        >> i.mPropertiesMap[s_email]
        >> i.mPropertiesMap[s_replyto]
        >> i.mPropertiesMap[s_bcc]
        >> i.mPropertiesMap[s_vcard]
        >> i.mPropertiesMap[s_transport]
        >> i.mPropertiesMap[s_fcc]
        >> i.mPropertiesMap[s_drafts]
        >> i.mPropertiesMap[s_templates]
        >> i.mPropertiesMap[s_signature]
        >> i.mPropertiesMap[s_dict]
        >> i.mPropertiesMap[s_xface]
        >> i.mPropertiesMap[s_prefcrypt];
   i.setProperty(s_uoid, uoid);
  return stream;
}

bool Identity::mailingAllowed() const
{
  return !property(s_email).toString().isEmpty();
}

void Identity::setIsDefault( bool flag )
{
  mIsDefault = flag;
}

void Identity::setIdentityName( const QString & name )
{
  setProperty(s_identity, name);
}

void Identity::setFullName(const QString &str)
{
  setProperty(s_name, str);
}

void Identity::setOrganization(const QString &str)
{
  setProperty(s_organization, str);
}

void Identity::setPGPSigningKey(const QByteArray &str)
{
  setProperty(s_pgps, QString( str ));
}

void Identity::setPGPEncryptionKey(const QByteArray &str)
{
  setProperty(s_pgpe, QString( str ));
}

void Identity::setSMIMESigningKey(const QByteArray &str)
{
  setProperty(s_smimes, QString( str ));
}

void Identity::setSMIMEEncryptionKey(const QByteArray &str)
{
  setProperty(s_smimee,  QString( str ) );
}

void Identity::setEmailAddr(const QString &str)
{
  setProperty(s_email, str);
}

void Identity::setVCardFile(const QString &str)
{
  setProperty(s_vcard, str);
}

QString Identity::fullEmailAddr(void) const
{
  const QString name = mPropertiesMap.value(s_name).toString();
  const QString mail = mPropertiesMap.value(s_email).toString();

  if (name.isEmpty())
      return mail;

  const QString specials("()<>@,.;:[]");

  QString result;

  // add DQUOTE's if necessary:
  bool needsQuotes=false;
  for (int i=0; i < name.length(); i++) {
    if ( specials.contains( name[i] ) )
      needsQuotes = true;
    else if ( name[i] == '\\' || name[i] == '"' ) {
      needsQuotes = true;
      result += '\\';
    }
    result += name[i];
  }

  if (needsQuotes) {
    result.insert(0,'"');
    result += '"';
  }

  result += " <" + mail + '>';

  return result;
}

void Identity::setReplyToAddr(const QString& str)
{
  setProperty(s_replyto, str);
}

void Identity::setSignatureFile(const QString &str)
{
  mSignature.setUrl( str, signatureIsCommand() );
}

void Identity::setSignatureInlineText(const QString &str )
{
  mSignature.setText( str );
}

void Identity::setTransport(const QString &str)
{
  setProperty(s_transport, str);
}

void Identity::setFcc(const QString &str)
{
  setProperty(s_fcc, str);
}

void Identity::setDrafts(const QString &str)
{
  setProperty(s_drafts, str);
}

void Identity::setTemplates(const QString &str)
{
  setProperty(s_templates, str);
}

void Identity::setDictionary( const QString &str )
{
  setProperty(s_dict, str);
}

void Identity::setBcc(const QString& str)
{
  setProperty(s_bcc, str);
}

void Identity::setPreferredCryptoMessageFormat( const QString& str)
{
  setProperty(s_prefcrypt, str);
}

void Identity::setXFace( const QString &str )
{
  // TODO: maybe make this non const, to indicate we actually are changing str
  QString strNew = str;
  strNew.remove( " " );
  strNew.remove( "\n" );
  strNew.remove( "\r" );
  setProperty(s_xface, strNew);
}

void Identity::setXFaceEnabled( const bool on )
{
  setProperty(s_xfaceenabled, on);
}

QString Identity::signatureText( bool * ok ) const
{
  bool internalOK = false;
  QString signatureText = mSignature.withSeparator( &internalOK );
  if ( internalOK ) {
    if ( ok )
        *ok=true;
    return signatureText;
  }

  // OK, here comes the funny part. The call to
  // Signature::withSeparator() failed, so we should probably fix the
  // cause:
  if ( ok )
      *ok = false;
  return QString();

#if 0 // TODO: error handling
  if (mSignatureFile.endsWith('|'))
  {
  }
  else
  {
  }
#endif

  return QString();
}


QString Identity::mimeDataType()
{
  return "application/x-kmail-identity-drag";
}

bool Identity::canDecode( const QMimeData*md )
{
  return md->hasFormat( mimeDataType() );
}

void Identity::populateMimeData( QMimeData*md )
{
  QByteArray a;
  {
    QDataStream s( &a, QIODevice::WriteOnly );
    s << this;
  }
  md->setData( mimeDataType(), a );
}

Identity Identity::fromMimeData( const QMimeData*md )
{
  Identity i;
  if ( canDecode( md ) ) {
    QByteArray ba = md->data( mimeDataType() );
    QDataStream s( &ba, QIODevice::ReadOnly );
    s >> i;
  }
  return i;
}

QVariant Identity::property( const QString & key ) const
{
  return mPropertiesMap.value(key);
}

void Identity::setProperty( const QString & key, const QVariant & value )
{
    if ( value.isNull() ||
         ( value.type() == QVariant::String && value.toString().isEmpty() ) )
      mPropertiesMap.remove( key );
  else
      mPropertiesMap.insert( key, value );    
}
