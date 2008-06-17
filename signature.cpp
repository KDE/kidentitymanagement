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

/**
 *   Private class that helps to provide binary compatibility between releases.
 *   @internal
 */
//@cond PRIVATE
class KPIMIdentities::Signature::Private
{
  public:
    QString textFromFile( bool *ok ) const;
    QString textFromCommand( bool *ok ) const;

    QString mUrl;
    QString mText;
    Type    mType;
    bool mInlinedHtml;
};

QString Signature::Private::textFromCommand( bool *ok ) const
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
    QString wmsg = i18n( "<qt>Failed to execute signature script<p><b>%1</b>:</p>"
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

QString Signature::Private::textFromFile( bool *ok ) const
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

//@endcond

Signature::Signature()
  : d( new Private )
{
    d->mType = Disabled;
    d->mInlinedHtml = false;
}

Signature::Signature( const QString &text )
  : d( new Private )
{
    d->mText = text;
    d->mType = Inlined;
    d->mInlinedHtml = false;
}

Signature::Signature( const QString &url, bool isExecutable )
  : d( new Private )
{
    d->mUrl = url;
    d->mType = isExecutable ? FromCommand : FromFile;
    d->mInlinedHtml = false;
}

Signature::Signature( const Signature &other )
  : d( new Private )
{
    *d = *other.d;
}

Signature::~Signature()
{
  delete d;
}

QString Signature::rawText( bool *ok ) const
{
  switch ( d->mType ) {
  case Disabled:
    if ( ok ) {
      *ok = true;
    }
    return QString();
  case Inlined:
    if ( ok ) {
      *ok = true;
    }
    return d->mText;
  case FromFile:
    return d->textFromFile( ok );
  case FromCommand:
    return d->textFromCommand( ok );
  };
  kFatal(5325) << "Signature::type() returned unknown value!";
  return QString(); // make compiler happy
}

QString Signature::withSeparator( bool *ok ) const
{
  QString signature = rawText( ok );
  if ( ok && (*ok) == false )
    return QString();

  if ( signature.isEmpty() ) {
    return signature; // don't add a separator in this case
  }

  QString newline = ( isInlinedHtml() && d->mType == Inlined ) ? "<br>" : "\n";
  if ( signature.startsWith( QString::fromLatin1( "-- " ) + newline )
    || ( signature.indexOf( newline + QString::fromLatin1( "-- " ) +
                            newline ) != -1 ) ) {
    // already have signature separator at start of sig or inside sig:
    return signature;
  } else {
    // need to prepend one:
    return QString::fromLatin1( "-- " ) + newline + signature;
  }
}

void Signature::setUrl( const QString &url, bool isExecutable )
{
  d->mUrl = url;
  d->mType = isExecutable ? FromCommand : FromFile;
}

void Signature::setInlinedHtml( bool isHtml )
{
  d->mInlinedHtml = isHtml;
}

bool Signature::isInlinedHtml() const
{
  return d->mInlinedHtml;
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
    d->mType = Inlined;
    d->mInlinedHtml = config.readEntry( sigTypeInlinedHtmlKey, false );
  } else if ( sigType == sigTypeFileValue ) {
    d->mType = FromFile;
    d->mUrl = config.readPathEntry( sigFileKey, QString() );
  } else if ( sigType == sigTypeCommandValue ) {
    d->mType = FromCommand;
    d->mUrl = config.readPathEntry( sigCommandKey, QString() );
  } else {
    d->mType = Disabled;
  }
  d->mText = config.readEntry( sigTextKey );
}

void Signature::writeConfig( KConfigGroup &config ) const
{
  switch ( d->mType ) {
    case Inlined:
      config.writeEntry( sigTypeKey, sigTypeInlineValue );
      config.writeEntry( sigTypeInlinedHtmlKey, d->mInlinedHtml );
      break;
    case FromFile:
      config.writeEntry( sigTypeKey, sigTypeFileValue );
      config.writePathEntry( sigFileKey, d->mUrl );
      break;
    case FromCommand:
      config.writeEntry( sigTypeKey, sigTypeCommandValue );
      config.writePathEntry( sigCommandKey, d->mUrl );
      break;
    case Disabled:
      config.writeEntry( sigTypeKey, sigTypeDisabledValue );
    default:
      break;
  }
  config.writeEntry( sigTextKey, d->mText );
}

// --------------------- Operators -------------------//

QDataStream &KPIMIdentities::operator<<
( QDataStream &stream, const KPIMIdentities::Signature &sig )
{
  return stream << static_cast<quint8>( sig.d->mType ) << sig.d->mUrl << sig.d->mText;
}

QDataStream &KPIMIdentities::operator>>
( QDataStream &stream, KPIMIdentities::Signature &sig )
{
  quint8 s;
  stream >> s  >> sig.d->mUrl >> sig.d->mText;
  sig.d->mType = static_cast<Signature::Type>( s );
  return stream;
}

bool Signature::operator== ( const Signature &other ) const
{
  if ( d->mType != other.d->mType ) {
    return false;
  }

  switch ( d->mType ) {
  case Inlined:
    return d->mText == other.d->mText;
  case FromFile:
  case FromCommand:
    return d->mUrl == other.d->mUrl;
  default:
  case Disabled:
    return true;
  }
}

Signature Signature::operator=( const Signature &other )
{
    *d = *other.d;
    return *this;
}

// --------------- Getters -----------------------//

QString Signature::text() const
{
  return d->mText;
}

QString Signature::url() const
{
  return d->mUrl;
}

Signature::Type Signature::type() const
{
  return d->mType;
}

// --------------- Setters -----------------------//

void Signature::setText( const QString &text )
{
  d->mText = text;
}

void Signature::setType( Type type )
{
  d->mType = type;
}
