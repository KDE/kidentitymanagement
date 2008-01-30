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

#include "signature.h"

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kconfiggroup.h>
#include <kurl.h>
#include <kprocess.h>
#include <kpimutils/kfileio.h>

#include <QFileInfo>
#include <assert.h>

using namespace KPIMIdentities;

Signature::Signature()
  : mType( Disabled )
{}

Signature::Signature( const QString &text )
  : mText( text ),
    mType( Inlined )
{}

Signature::Signature( const QString &url, bool isExecutable )
  : mUrl( url ),
    mType( isExecutable ? FromCommand : FromFile )
{}

QString Signature::rawText( bool *ok ) const
{
  switch ( mType ) {
  case Disabled:
    if ( ok ) {
      *ok = true;
    }
    return QString();
  case Inlined:
    if ( ok ) {
      *ok = true;
    }
    return mText;
  case FromFile:
    return textFromFile( ok );
  case FromCommand:
    return textFromCommand( ok );
  };
  kFatal(5325) << "Signature::type() returned unknown value!";
  return QString(); // make compiler happy
}

QString Signature::textFromCommand( bool *ok ) const
{
  assert( mType == FromCommand );

  // handle pathological cases:
  if ( mUrl.isEmpty() ) {
    if ( ok ) {
      *ok = true;
    }
    return QString();
  }

  // create a shell process:
  KProcess proc;
  proc.setOutputChannelMode( KProcess::SeparateChannels );
  proc.setShellCommand( mUrl );
  int rc = proc.execute();

  // handle errors, if any:
  if ( rc != 0 ) {
    if ( ok ) {
      *ok = false;
    }
    QString wmsg = i18n( "<qt>Failed to execute signature script<br><b>%1</b>:"
                         "<p>%2</p></qt>", mUrl, QString( proc.readAllStandardError() ) );
    KMessageBox::error( 0, wmsg );
    return QString();
  }

  // no errors:
  if ( ok ) {
    *ok = true;
  }

  // get output:
  QByteArray output = proc.readAllStandardOutput();

  // TODO: hmm, should we allow other encodings, too?
  return QString::fromLocal8Bit( output.data(), output.size() );
}

QString Signature::textFromFile( bool *ok ) const
{
  assert( mType == FromFile );

  // TODO: Use KIO::NetAccess to download non-local files!
  if ( !KUrl( mUrl ).isLocalFile() &&
       !( QFileInfo( mUrl ).isRelative() &&
          QFileInfo( mUrl ).exists() ) ) {
    kDebug(5325) << "Signature::textFromFile:"
    << "non-local URLs are unsupported";
    if ( ok ) {
      *ok = false;
    }
    return QString();
  }

  if ( ok ) {
    *ok = true;
  }

  // TODO: hmm, should we allow other encodings, too?
  const QByteArray ba = KPIMUtils::kFileToByteArray( mUrl, false );
  return QString::fromLocal8Bit( ba.data(), ba.size() );
}

QString Signature::withSeparator( bool *ok ) const
{
  bool internalOK = false;
  QString signature = rawText( &internalOK );
  if ( !internalOK ) {
    if ( ok ) {
      *ok = false;
    }
    return QString();
  }
  if ( ok ) {
    *ok = true;
  }

  if ( signature.isEmpty() ) {
    return signature; // don't add a separator in this case
  }

  if ( signature.startsWith( QString::fromLatin1( "-- \n" ) ) ) {
    // already have signature separator at start of sig:
    return signature;
  } else {
    // need to prepend one:
    return QString::fromLatin1( "-- \n" ) + signature;
  }
}

void Signature::setUrl( const QString &url, bool isExecutable )
{
  mUrl = url;
  mType = isExecutable ? FromCommand : FromFile;
}

void Signature::setInlinedHtml( bool isHtml )
{
  mInlinedHtml = isHtml;
}

bool Signature::isInlinedHtml() const
{
  return mInlinedHtml;
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
static const char sigTypeInlinedHtmlKey[] = "Inlined Html";

void Signature::readConfig( const KConfigGroup &config )
{
  QString sigType = config.readEntry( sigTypeKey );
  if ( sigType == sigTypeInlineValue ) {
    mType = Inlined;
    mInlinedHtml = config.readEntry( sigTypeInlinedHtmlKey, false );
  } else if ( sigType == sigTypeFileValue ) {
    mType = FromFile;
    mUrl = config.readPathEntry( sigFileKey, QString() );
  } else if ( sigType == sigTypeCommandValue ) {
    mType = FromCommand;
    mUrl = config.readPathEntry( sigCommandKey, QString() );
  } else {
    mType = Disabled;
  }
  mText = config.readEntry( sigTextKey );
}

void Signature::writeConfig( KConfigGroup &config ) const
{
  switch ( mType ) {
    case Inlined:
      config.writeEntry( sigTypeKey, sigTypeInlineValue );
      config.writeEntry( sigTypeInlinedHtmlKey, mInlinedHtml );
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
    default:
      break;
  }
  config.writeEntry( sigTextKey, mText );
}

// --------------------- Operators -------------------//

QDataStream &KPIMIdentities::operator<<
( QDataStream &stream, const KPIMIdentities::Signature &sig )
{
  return stream << static_cast<quint8>( sig.mType ) << sig.mUrl << sig.mText;
}

QDataStream &KPIMIdentities::operator>>
( QDataStream &stream, KPIMIdentities::Signature &sig )
{
  quint8 s;
  stream >> s  >> sig.mUrl >> sig.mText;
  sig.mType = static_cast<Signature::Type>( s );
  return stream;
}

bool Signature::operator== ( const Signature &other ) const
{
  if ( mType != other.mType ) {
    return false;
  }

  switch ( mType ) {
  case Inlined:
    return mText == other.mText;
  case FromFile:
  case FromCommand:
    return mUrl == other.mUrl;
  default:
  case Disabled:
    return true;
  }
}

// --------------- Getters -----------------------//

QString Signature::text() const
{
  return mText;
}

QString Signature::url() const
{
  return mUrl;
}

Signature::Type Signature::type() const
{
  return mType;
}

// --------------- Setters -----------------------//

void Signature::setText( const QString &text )
{
  mText = text;
}

void Signature::setType( Type type )
{
  mType = type;
}
