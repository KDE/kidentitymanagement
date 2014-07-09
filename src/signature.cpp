/*
    Copyright (c) 2002-2004 Marc Mutz <mutz@kde.org>
    Copyright (c) 2007 Tom Albers <tomalbers@kde.nl>
    Copyright (c) 2009 Thomas McGuire <mcguire@kde.org>

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

#include <qdebug.h>
#include <klocalizedstring.h>
#include <kmessagebox.h>
#include <kconfiggroup.h>
#include <QUrl>
#include <kprocess.h>
#include <KRichTextEdit>
#include <kpimutils/kfileio.h>

#include <QFileInfo>
#include <QSharedPointer>
#include <QImage>

#include <assert.h>
#include <QtCore/QDir>
#include <kpimtextedit/textedit.h>

using namespace KPIMIdentities;

class SignaturePrivate
{
  public:
    SignaturePrivate()
      :enabled( false )
    {
    }
    struct EmbeddedImage
    {
      QImage image;
      QString name;
    };
    typedef QSharedPointer<EmbeddedImage> EmbeddedImagePtr;

    /// List of images that belong to this signature. Either added by addImage() or
    /// by readConfig().
    QList<EmbeddedImagePtr> embeddedImages;

    /// The directory where the images will be saved to.
    QString saveLocation;
    bool enabled;
};

QDataStream &operator<< ( QDataStream &stream, const SignaturePrivate::EmbeddedImagePtr &img )
{
  return stream << img->image << img->name;
}

QDataStream &operator>> ( QDataStream &stream, SignaturePrivate::EmbeddedImagePtr &img )
{
  return stream >> img->image >> img->name;
}

// TODO: KDE5: BIC: Add a real d-pointer.
// This QHash is just a workaround around BIC issues, for more info see
// http://techbase.kde.org/Policies/Binary_Compatibility_Issues_With_C++
typedef QHash<const Signature*,SignaturePrivate*> SigPrivateHash;
Q_GLOBAL_STATIC( SigPrivateHash, d_func )

static SignaturePrivate* d( const Signature *sig )
{
  SignaturePrivate *ret = d_func()->value( sig, 0 );
  if ( !ret ) {
    ret = new SignaturePrivate;
    d_func()->insert( sig, ret );
  }
  return ret;
}

static void delete_d( const Signature* sig )
{
  SignaturePrivate *ret = d_func()->value( sig, 0 );
  delete ret;
  d_func()->remove( sig );
}

Signature::Signature()
  : mType( Disabled ),
    mInlinedHtml( false )
{}

Signature::Signature( const QString &text )
  : mText( text ),
    mType( Inlined ),
    mInlinedHtml( false )
{}

Signature::Signature( const QString &url, bool isExecutable )
  : mUrl( url ),
    mType( isExecutable ? FromCommand : FromFile ),
    mInlinedHtml( false )
{}

void Signature::assignFrom ( const KPIMIdentities::Signature &that )
{
  mUrl = that.mUrl;
  mInlinedHtml = that.mInlinedHtml;
  mText = that.mText;
  mType = that.mType;
  d( this )->enabled = d( &that )->enabled;
  d( this )->saveLocation = d( &that )->saveLocation;
  d( this )->embeddedImages = d( &that )->embeddedImages;
}

Signature::Signature ( const Signature &that )
{
  assignFrom( that );
}

Signature& Signature::operator= ( const KPIMIdentities::Signature & that )
{
  if ( this == &that ) {
    return *this;
  }

  assignFrom( that );
  return *this;
}

Signature::~Signature()
{
  delete_d( this );
}

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
  qCritical() << "Signature::type() returned unknown value!";
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
    const QString wmsg = i18n( "<qt>Failed to execute signature script<p><b>%1</b>:</p>"
                         "<p>%2</p></qt>", mUrl, QLatin1String( proc.readAllStandardError() ) );
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
  if ( !QUrl( mUrl ).isLocalFile() &&
       !( QFileInfo( mUrl ).isRelative() &&
          QFileInfo( mUrl ).exists() ) ) {
    qDebug() << "Signature::textFromFile:"
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
  QString signature = rawText( ok );
  if ( ok && ( *ok ) == false ) {
    return QString();
  }

  if ( signature.isEmpty() ) {
    return signature; // don't add a separator in this case
  }

  const bool htmlSig = ( isInlinedHtml() && mType == Inlined );
  QString newline = htmlSig ? QLatin1String("<br>") : QLatin1String("\n");
  if ( htmlSig && signature.startsWith( QLatin1String( "<p" ) ) ) {
    newline.clear();
  }

  if ( signature.startsWith( QString::fromLatin1( "-- " ) + newline ) ||
       ( signature.indexOf( newline + QString::fromLatin1( "-- " ) + newline ) != -1 ) ) {
    // already have signature separator at start of sig or inside sig:
    return signature;
  } else {
    // need to prepend one:
    return QString::fromLatin1( "-- " ) + newline + signature;
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
static const char sigImageLocation[] = "Image Location";
static const char sigEnabled[] ="Signature Enabled";

// Returns the names of all images in the HTML code
static QStringList findImageNames( const QString &htmlCode )
{
  QStringList ret;

  // To complicated for us, so cheat and let a text edit do the hard work
  KPIMTextEdit::TextEdit edit;
  edit.setHtml( htmlCode );
  foreach ( const KPIMTextEdit::ImageWithNamePtr &image, edit.imagesWithName() ) {
    ret << image->name;
  }
  return ret;
}

void Signature::cleanupImages() const
{
  // Remove any images from the internal structure that are no longer there
  if ( isInlinedHtml() ) {
    foreach ( const SignaturePrivate::EmbeddedImagePtr &imageInList, d( this )->embeddedImages ) {
      bool found = false;
      foreach ( const QString &imageInHtml, findImageNames( mText ) ) {
        if ( imageInHtml == imageInList->name ) {
          found = true;
          break;
        }
      }
      if ( !found ) {
        d( this )->embeddedImages.removeAll( imageInList );
      }
    }
  }

  // Delete all the old image files
  if ( !d( this )->saveLocation.isEmpty() ) {
    QDir dir( d( this )->saveLocation );
    foreach ( const QString &fileName, dir.entryList( QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks ) ) {
      if ( fileName.toLower().endsWith( QLatin1String( ".png" ) ) ) {
        qDebug() << "Deleting old image" << dir.path() + fileName;
        dir.remove( fileName );
      }
    }
  }
}

void Signature::saveImages() const
{
  if ( isInlinedHtml() && !d( this )->saveLocation.isEmpty() ) {
    foreach ( const SignaturePrivate::EmbeddedImagePtr &image, d( this )->embeddedImages ) {
      QString location = d( this )->saveLocation + QLatin1Char('/') + image->name;
      if ( !image->image.save( location, "PNG" ) ) {
        qWarning() << "Failed to save image" << location;
      }
    }
  }
}

void Signature::readConfig( const KConfigGroup &config )
{
  QString sigType = config.readEntry( sigTypeKey );
  if ( sigType == QLatin1String(sigTypeInlineValue) ) {
    mType = Inlined;
    mInlinedHtml = config.readEntry( sigTypeInlinedHtmlKey, false );
  } else if ( sigType == QLatin1String(sigTypeFileValue) ) {
    mType = FromFile;
    mUrl = config.readPathEntry( sigFileKey, QString() );
  } else if ( sigType == QLatin1String(sigTypeCommandValue) ) {
    mType = FromCommand;
    mUrl = config.readPathEntry( sigCommandKey, QString() );
  } else if ( sigType == QLatin1String(sigTypeDisabledValue) ) {
    d( this )->enabled = false;
  }
  if ( mType != Disabled ) {
    d( this )->enabled = config.readEntry( sigEnabled, true );
  }

  mText = config.readEntry( sigTextKey );
  d( this )->saveLocation = config.readEntry( sigImageLocation );

  if ( isInlinedHtml() && !d( this )->saveLocation.isEmpty() ) {
    QDir dir( d( this )->saveLocation );
    foreach ( const QString &fileName, dir.entryList( QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks ) ) {
      if ( fileName.toLower().endsWith( QLatin1String( ".png" ) ) ) {
        QImage image;
        if ( image.load( dir.path() + QLatin1Char('/') + fileName ) ) {
          addImage( image, fileName );
        }
        else {
          qWarning() << "Unable to load image" << dir.path() + QLatin1Char('/') + fileName;
        }
      }
    }
  }
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
    default:
      break;
  }
  config.writeEntry( sigTextKey, mText );
  config.writeEntry( sigImageLocation, d( this )->saveLocation );
  config.writeEntry( sigEnabled, d( this )->enabled );

  cleanupImages();
  saveImages();
}

static bool isCursorAtEndOfLine( const QTextCursor &cursor )
{
  QTextCursor testCursor = cursor;
  testCursor.movePosition( QTextCursor::EndOfLine, QTextCursor::KeepAnchor );
  return !testCursor.hasSelection();
}

static void insertSignatureHelper( const QString &signature,
                                   KRichTextEdit *textEdit,
                                   Signature::Placement placement,
                                   bool isHtml,
                                   bool addNewlines )
{
  if ( !signature.isEmpty() ) {

    // Save the modified state of the document, as inserting a signature
    // shouldn't change this. Restore it at the end of this function.
    bool isModified = textEdit->document()->isModified();

    // Move to the desired position, where the signature should be inserted
    QTextCursor cursor = textEdit->textCursor();
    QTextCursor oldCursor = cursor;
    cursor.beginEditBlock();

    if ( placement == Signature::End ) {
      cursor.movePosition( QTextCursor::End );
    } else if ( placement == Signature::Start ) {
      cursor.movePosition( QTextCursor::Start );
    } else if ( placement == Signature::AtCursor ) {
      cursor.movePosition( QTextCursor::StartOfLine );
    }
    textEdit->setTextCursor( cursor );


    QString lineSep;
    if ( addNewlines ) {
      if ( isHtml ) {
        lineSep = QLatin1String( "<br>" );
      } else {
        lineSep = QLatin1Char( '\n' );
      }
    }

    // Insert the signature and newlines depending on where it was inserted.
    bool hackForCursorsAtEnd = false;
    int oldCursorPos = -1;
    if ( placement == Signature::End ) {

      if ( oldCursor.position() == textEdit->toPlainText().length() ) {
        hackForCursorsAtEnd = true;
        oldCursorPos = oldCursor.position();
      }
      if ( isHtml ) {
        textEdit->insertHtml( lineSep + signature );
      } else {
        textEdit->insertPlainText( lineSep + signature );
      }
    } else if ( placement == Signature::Start || placement == Signature::AtCursor ) {
      if ( isHtml ) {
        if ( isCursorAtEndOfLine( cursor ) ) {
          textEdit->insertHtml( signature );
        } else {
          textEdit->insertHtml( signature + lineSep );
        }
      } else {
        if ( isCursorAtEndOfLine( cursor ) ) {
          textEdit->insertPlainText( signature );
        } else {
          textEdit->insertPlainText( signature + lineSep );
        }
      }
    }

    cursor.endEditBlock();

    // There is one special case when re-setting the old cursor: The cursor
    // was at the end. In this case, QTextEdit has no way to know
    // if the signature was added before or after the cursor, and just decides
    // that it was added before (and the cursor moves to the end, but it should
    // not when appending a signature). See bug 167961
    if ( hackForCursorsAtEnd ) {
      oldCursor.setPosition( oldCursorPos );
    }

    textEdit->setTextCursor( oldCursor );
    textEdit->ensureCursorVisible();

    textEdit->document()->setModified( isModified );

    if ( isHtml ) {
      textEdit->enableRichTextMode();
    }
  }
}

void Signature::insertIntoTextEdit( KRichTextEdit *textEdit,
                                    Placement placement, bool addSeparator )
{
  if ( !isEnabledSignature() ) {
    return;
  }
  QString signature;
  if ( addSeparator ) {
    signature = withSeparator();
  } else {
    signature = rawText();
  }
  insertSignatureHelper( signature, textEdit, placement,
                   ( isInlinedHtml() &&
                     type() == KPIMIdentities::Signature::Inlined ),
                   true );
}

void Signature::insertIntoTextEdit( Placement placement, AddedText addedText,
                                    KPIMTextEdit::TextEdit *textEdit, bool forceDisplay ) const
{
  insertSignatureText( placement, addedText, textEdit, forceDisplay );
}

void Signature::insertSignatureText(Placement placement, AddedText addedText, KPIMTextEdit::TextEdit *textEdit, bool forceDisplay) const
{
  if ( !forceDisplay ) {
    if ( !isEnabledSignature() ) {
      return;
    }
  }
  QString signature;
  if ( addedText & AddSeparator ) {
    signature = withSeparator();
  } else {
    signature = rawText();
  }
  insertSignatureHelper( signature, textEdit, placement,
                         ( isInlinedHtml() &&
                           type() == KPIMIdentities::Signature::Inlined ),
                         ( addedText & AddNewLines ) );

  // We added the text of the signature above, now it is time to add the images as well.
  if ( isInlinedHtml() ) {
    foreach ( const SignaturePrivate::EmbeddedImagePtr &image, d( this )->embeddedImages ) {
      textEdit->loadImage( image->image, image->name, image->name );
    }
  }
}


void Signature::insertPlainSignatureIntoTextEdit( const QString &signature, KRichTextEdit *textEdit,
                                                  Signature::Placement placement, bool isHtml )
{
  insertSignatureHelper( signature, textEdit, placement, isHtml, true );
}

// --------------------- Operators -------------------//

QDataStream &KPIMIdentities::operator<<
( QDataStream &stream, const KPIMIdentities::Signature &sig )
{
  return stream << static_cast<quint8>( sig.mType ) << sig.mUrl << sig.mText
                << d( &sig )->saveLocation << d( &sig )->embeddedImages << d( &sig )->enabled;
}

QDataStream &KPIMIdentities::operator>>
( QDataStream &stream, KPIMIdentities::Signature &sig )
{
  quint8 s;
  stream >> s  >> sig.mUrl >> sig.mText >> d( &sig )->saveLocation >> d( &sig )->embeddedImages >>d( &sig )->enabled;
  sig.mType = static_cast<Signature::Type>( s );
  return stream;
}

bool Signature::operator== ( const Signature &other ) const
{
  if ( mType != other.mType ) {
    return false;
  }

  if ( d( this )->enabled != d( &other )->enabled ) {
    return false;
  }

  if ( mType == Inlined && mInlinedHtml ) {
    if ( d( this )->saveLocation != d( &other )->saveLocation ) {
      return false;
    }
    if ( d( this )->embeddedImages != d( &other )->embeddedImages ) {
      return false;
    }
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

QString Signature::toPlainText() const
{
  QString sigText = rawText();
  if ( !sigText.isEmpty() && isInlinedHtml() && type() == Inlined ) {
    // Use a QTextDocument as a helper, it does all the work for us and
    // strips all HTML tags.
    QTextDocument helper;
    QTextCursor helperCursor( &helper );
    helperCursor.insertHtml( sigText );
    sigText = helper.toPlainText();
  }
  return sigText;
}

void Signature::addImage ( const QImage& imageData, const QString& imageName )
{
  Q_ASSERT( !( d( this )->saveLocation.isEmpty() ) );
  SignaturePrivate::EmbeddedImagePtr image( new SignaturePrivate::EmbeddedImage() );
  image->image = imageData;
  image->name = imageName;
  d( this )->embeddedImages.append( image );
}

void Signature::setImageLocation ( const QString& path )
{
  d( this )->saveLocation = path;
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
  mType = Inlined;
}

void Signature::setType( Type type )
{
  mType = type;
}


void Signature::setEnabledSignature(bool enabled)
{
  d( this )->enabled = enabled;
}

bool Signature::isEnabledSignature() const
{
  return d( this )->enabled;
}
