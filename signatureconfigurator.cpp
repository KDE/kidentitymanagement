/*  -*- c++ -*-
    Copyright 2008 Thomas McGuire <Thomas.McGuire@gmx.net>
    Copyright 2008 Edwin Schepers <yez@familieschepers.nl>
    Copyright 2004 Marc Mutz <mutz@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "signatureconfigurator.h"
#include "identity.h"

#include <kactioncollection.h>
#include <klocalizedstring.h>
#include <kdebug.h>
#include <kdialog.h>
#include <klineedit.h>
#include <kurlrequester.h>
#include <kshellcompletion.h>
#include <ktoolbar.h>
#include <krun.h>
#include <KUrl>
#include <KComboBox>
#include <KStandardDirs>

#include <kpimtextedit/textedit.h>

#include <QCheckBox>
#include <QDir>
#include <QFileInfo>
#include <QLabel>
#include <QLayout>
#include <QMimeData>
#include <QTextEdit>

#include <QStackedWidget>

#include <QVBoxLayout>
#include <QHBoxLayout>

#include <assert.h>

using namespace KPIMIdentities;

namespace KPIMIdentities {

/**
   Private class that helps to provide binary compatibility between releases.
   @internal
  */
//@cond PRIVATE
class SignatureConfigurator::Private
{
  public:
    Private( SignatureConfigurator *parent );
    void init();

    SignatureConfigurator *q;
    bool inlinedHtml;
    QString imageLocation;
};
//@endcond

SignatureConfigurator::Private::Private( SignatureConfigurator *parent )
  :q( parent ), inlinedHtml( true )
{
}

void SignatureConfigurator::Private::init()
{
  // tmp. vars:
  QLabel * label;
  QWidget * page;
  QHBoxLayout * hlay;
  QVBoxLayout * vlay;
  QVBoxLayout * page_vlay;

  vlay = new QVBoxLayout( q );
  vlay->setObjectName( QLatin1String("main layout") );
  vlay->setMargin( 0 );

  // "enable signatue" checkbox:
  q->mEnableCheck = new QCheckBox( i18n( "&Enable signature" ), q );
  q->mEnableCheck->setWhatsThis(
      i18n( "Check this box if you want KMail to append a signature to mails "
            "written with this identity." ) );
  vlay->addWidget( q->mEnableCheck );

  // "obtain signature text from" combo and label:
  hlay = new QHBoxLayout(); // inherits spacing
  vlay->addLayout( hlay );
  q->mSourceCombo = new KComboBox( q );
  q->mSourceCombo->setEditable( false );
  q->mSourceCombo->setWhatsThis(
      i18n( "Click on the widgets below to obtain help on the input methods." ) );
  q->mSourceCombo->setEnabled( false ); // since !mEnableCheck->isChecked()
  q->mSourceCombo->addItems( QStringList()
                  << i18nc( "continuation of \"obtain signature text from\"",
                            "Input Field Below" )
                  << i18nc( "continuation of \"obtain signature text from\"",
                            "File" )
                  << i18nc( "continuation of \"obtain signature text from\"",
                            "Output of Command" ) );
  label = new QLabel( i18n( "Obtain signature &text from:" ), q );
  label->setBuddy( q->mSourceCombo );
  label->setEnabled( false ); // since !mEnableCheck->isChecked()
  hlay->addWidget( label );
  hlay->addWidget( q->mSourceCombo, 1 );

  // widget stack that is controlled by the source combo:
  QStackedWidget * widgetStack = new QStackedWidget( q );
  widgetStack->setEnabled( false ); // since !mEnableCheck->isChecked()
  vlay->addWidget( widgetStack, 1 );
  q->connect( q->mSourceCombo, SIGNAL(currentIndexChanged(int)),
              widgetStack, SLOT(setCurrentIndex(int)) );
  q->connect( q->mSourceCombo, SIGNAL(highlighted(int)),
              widgetStack, SLOT(setCurrentIndex(int)) );
  // connects for the enabling of the widgets depending on
  // signatureEnabled:
  q->connect( q->mEnableCheck, SIGNAL(toggled(bool)),
              q->mSourceCombo, SLOT(setEnabled(bool)) );
  q->connect( q->mEnableCheck, SIGNAL(toggled(bool)),
              widgetStack, SLOT(setEnabled(bool)) );
  q->connect( q->mEnableCheck, SIGNAL(toggled(bool)),
              label, SLOT(setEnabled(bool)) );
  // The focus might be still in the widget that is disabled
  q->connect( q->mEnableCheck, SIGNAL(clicked()),
              q->mEnableCheck, SLOT(setFocus()) );

  int pageno = 0;
  // page 0: input field for direct entering:
  page = new QWidget( widgetStack );
  widgetStack->insertWidget( pageno, page );
  page_vlay = new QVBoxLayout( page );

#ifndef QT_NO_TOOLBAR
  q->mEditToolBar = new KToolBar( q );
  q->mEditToolBar->setToolButtonStyle( Qt::ToolButtonIconOnly );
  page_vlay->addWidget( q->mEditToolBar, 0 );

  q->mFormatToolBar = new KToolBar( q );
  q->mFormatToolBar->setToolButtonStyle( Qt::ToolButtonIconOnly );
  page_vlay->addWidget( q->mFormatToolBar, 1 );
#endif

  q->mTextEdit = new KPIMTextEdit::TextEdit( q );
  static_cast<KPIMTextEdit::TextEdit*>( q->mTextEdit )->enableImageActions();
  static_cast<KPIMTextEdit::TextEdit*>( q->mTextEdit )->enableInsertHtmlActions();
  static_cast<KPIMTextEdit::TextEdit*>( q->mTextEdit )->enableInsertTableActions();
  page_vlay->addWidget( q->mTextEdit, 2 );
  q->mTextEdit->setWhatsThis( i18n( "Use this field to enter an arbitrary static signature." ) );
  // exclude SupportToPlainText.
  q->mTextEdit->setRichTextSupport( KRichTextWidget::FullTextFormattingSupport |
      KRichTextWidget::FullListSupport |
      KRichTextWidget::SupportAlignment |
      KRichTextWidget::SupportRuleLine |
      KRichTextWidget::SupportHyperlinks |
      KRichTextWidget::SupportFormatPainting );

  // Fill the toolbars.
  KActionCollection *actionCollection = new KActionCollection( q );
  q->mTextEdit->createActions( actionCollection );
#ifndef QT_NO_TOOLBAR
  q->mEditToolBar->addAction( actionCollection->action( QLatin1String("format_text_bold") ) );
  q->mEditToolBar->addAction( actionCollection->action( QLatin1String("format_text_italic") ) );
  q->mEditToolBar->addAction( actionCollection->action( QLatin1String("format_text_underline") ) );
  q->mEditToolBar->addAction( actionCollection->action( QLatin1String("format_text_strikeout") ) );
  q->mEditToolBar->addAction( actionCollection->action( QLatin1String("format_text_foreground_color") ) );
  q->mEditToolBar->addAction( actionCollection->action( QLatin1String("format_text_background_color") ) );
  q->mEditToolBar->addAction( actionCollection->action( QLatin1String("format_font_family") ) );
  q->mEditToolBar->addAction( actionCollection->action( QLatin1String("format_font_size") ) );
  q->mEditToolBar->addAction( actionCollection->action( QLatin1String("format_reset") ) );

  q->mFormatToolBar->addAction( actionCollection->action( QLatin1String("format_list_style") ) );
  q->mFormatToolBar->addAction( actionCollection->action( QLatin1String("format_list_indent_more") ) );
  q->mFormatToolBar->addAction( actionCollection->action( QLatin1String("format_list_indent_less") ) );
  q->mFormatToolBar->addAction( actionCollection->action( QLatin1String("format_list_indent_less") ) );
  q->mFormatToolBar->addSeparator();

  q->mFormatToolBar->addAction( actionCollection->action( QLatin1String("format_align_left") ) );
  q->mFormatToolBar->addAction( actionCollection->action( QLatin1String("format_align_center") ) );
  q->mFormatToolBar->addAction( actionCollection->action( QLatin1String("format_align_right") ) );
  q->mFormatToolBar->addAction( actionCollection->action( QLatin1String("format_align_justify") ) );
  q->mFormatToolBar->addSeparator();

  q->mFormatToolBar->addAction( actionCollection->action( QLatin1String("insert_horizontal_rule") ) );
  q->mFormatToolBar->addAction( actionCollection->action( QLatin1String("manage_link") ) );
  q->mFormatToolBar->addAction( actionCollection->action( QLatin1String("format_painter") ) );

  q->mFormatToolBar->addSeparator();
  q->mFormatToolBar->addAction( actionCollection->action( QLatin1String("add_image") ) );
  q->mFormatToolBar->addSeparator();
  q->mFormatToolBar->addAction( actionCollection->action( QLatin1String("insert_html") ) );
  q->mFormatToolBar->addAction( actionCollection->action( QLatin1String("insert_table" )) );
#endif

  hlay = new QHBoxLayout(); // inherits spacing
  page_vlay->addLayout( hlay );
  q->mHtmlCheck = new QCheckBox( i18n( "&Use HTML" ), page );
  q->connect( q->mHtmlCheck, SIGNAL(clicked()),
              q, SLOT(slotSetHtml()) );
  hlay->addWidget( q->mHtmlCheck );
  inlinedHtml = true;

  widgetStack->setCurrentIndex( 0 ); // since mSourceCombo->currentItem() == 0

  // page 1: "signature file" requester, label, "edit file" button:
  ++pageno;
  page = new QWidget( widgetStack );
  widgetStack->insertWidget( pageno, page ); // force sequential numbers (play safe)
  page_vlay = new QVBoxLayout( page );
  page_vlay->setMargin( 0 );
  hlay = new QHBoxLayout(); // inherits spacing
  page_vlay->addLayout( hlay );
  q->mFileRequester = new KUrlRequester( page );
  q->mFileRequester->setWhatsThis(
      i18n( "Use this requester to specify a text file that contains your "
            "signature. It will be read every time you create a new mail or "
            "append a new signature." ) );
  label = new QLabel( i18n( "S&pecify file:" ), page );
  label->setBuddy( q->mFileRequester );
  hlay->addWidget( label );
  hlay->addWidget( q->mFileRequester, 1 );
  q->mFileRequester->button()->setAutoDefault( false );
  q->connect( q->mFileRequester, SIGNAL(textChanged(QString)),
              q, SLOT(slotEnableEditButton(QString)) );
  q->mEditButton = new QPushButton( i18n( "Edit &File" ), page );
  q->mEditButton->setWhatsThis( i18n( "Opens the specified file in a text editor." ) );
  q->connect( q->mEditButton, SIGNAL(clicked()),
              q, SLOT(slotEdit()) );
  q->mEditButton->setAutoDefault( false );
  q->mEditButton->setEnabled( false ); // initially nothing to edit
  hlay->addWidget( q->mEditButton );
  page_vlay->addStretch( 1 ); // spacer

  // page 2: "signature command" requester and label:
  ++pageno;
  page = new QWidget( widgetStack );
  widgetStack->insertWidget( pageno, page );
  page_vlay = new QVBoxLayout( page );
  page_vlay->setMargin( 0 );
  hlay = new QHBoxLayout(); // inherits spacing
  page_vlay->addLayout( hlay );
  q->mCommandEdit = new KLineEdit( page );
  q->mCommandEdit->setCompletionObject( new KShellCompletion() );
  q->mCommandEdit->setAutoDeleteCompletionObject( true );
  q->mCommandEdit->setWhatsThis(
      i18n( "You can add an arbitrary command here, either with or without path "
            "depending on whether or not the command is in your Path. For every "
            "new mail, KMail will execute the command and use what it outputs (to "
            "standard output) as a signature. Usual commands for use with this "
            "mechanism are \"fortune\" or \"ksig -random\"." ) );
  label = new QLabel( i18n( "S&pecify command:" ), page );
  label->setBuddy( q->mCommandEdit );
  hlay->addWidget( label );
  hlay->addWidget( q->mCommandEdit, 1 );
  page_vlay->addStretch( 1 ); // spacer
}

  SignatureConfigurator::SignatureConfigurator( QWidget * parent )
    : QWidget( parent ), d( new Private( this ) )
  {
    d->init();
  }

  SignatureConfigurator::~SignatureConfigurator()
  {
    delete d;
  }

  bool SignatureConfigurator::isSignatureEnabled() const
  {
    return mEnableCheck->isChecked();
  }

  void SignatureConfigurator::setSignatureEnabled( bool enable )
  {
    mEnableCheck->setChecked( enable );
  }

  Signature::Type SignatureConfigurator::signatureType() const
  {
    switch ( mSourceCombo->currentIndex() ) {
    case 0:  return Signature::Inlined;
    case 1:  return Signature::FromFile;
    case 2:  return Signature::FromCommand;
    default: return Signature::Disabled;
    }
  }

  void SignatureConfigurator::setSignatureType( Signature::Type type )
  {
    int idx = 0;
    switch ( type ) {
    case Signature::Inlined:     idx = 0; break;
    case Signature::FromFile:    idx = 1; break;
    case Signature::FromCommand: idx = 2; break;
    default:                     idx = 0; break;
    };

    mSourceCombo->setCurrentIndex( idx );
  }

  void SignatureConfigurator::setInlineText( const QString & text )
  {
    mTextEdit->setTextOrHtml( text );
  }

  QString SignatureConfigurator::fileURL() const
  {
    QString file = mFileRequester->url().path();

    // Force the filename to be relative to ~ instead of $PWD depending
    // on the rest of the code (KRun::run in Edit and KFileItem on save)
    if ( !file.isEmpty() && QFileInfo( file ).isRelative() ) {
      file = QDir::home().absolutePath() + QDir::separator() + file;
    }
    return file;
  }

  void SignatureConfigurator::setFileURL( const QString & url )
  {
    mFileRequester->setUrl( QUrl(url) );
  }

  QString SignatureConfigurator::commandURL() const
  {
    return mCommandEdit->text();
  }

  void SignatureConfigurator::setCommandURL( const QString & url )
  {
    mCommandEdit->setText( url );
  }


  Signature SignatureConfigurator::signature() const
  {
    Signature sig;
    const Signature::Type sigType = signatureType();
    switch ( sigType ) {
    case Signature::Inlined:
      sig.setInlinedHtml( d->inlinedHtml );
      sig.setText( d->inlinedHtml ? asCleanedHTML() : mTextEdit->textOrHtml() );
      if ( d->inlinedHtml ) {
        if ( !d->imageLocation.isEmpty() ) {
          sig.setImageLocation( d->imageLocation );
        }
        KPIMTextEdit::ImageWithNameList images = static_cast< KPIMTextEdit::TextEdit*>( mTextEdit )->imagesWithName();
        foreach ( const KPIMTextEdit::ImageWithNamePtr &image, images ) {
          sig.addImage( image->image, image->name );
        }
      }
      break;
    case Signature::FromCommand:
      sig.setUrl( commandURL(), true );
      break;
    case Signature::FromFile:
      sig.setUrl( fileURL(), false );
      break;
    case Signature::Disabled:
      /* do nothing */
      break;
    }
    sig.setEnabledSignature( isSignatureEnabled() );
    sig.setType( sigType );
    return sig;
  }

  void SignatureConfigurator::setSignature( const Signature & sig )
  {
    setSignatureType( sig.type() );
    setSignatureEnabled( sig.isEnabledSignature() );

    if ( sig.isInlinedHtml() ) {
      mHtmlCheck->setCheckState( Qt::Checked );
    } else {
      mHtmlCheck->setCheckState( Qt::Unchecked );
    }
    slotSetHtml();

    // Let insertIntoTextEdit() handle setting the text, as that function also adds the images.
    mTextEdit->clear();
    KPIMTextEdit::TextEdit * const pimEdit = static_cast<KPIMTextEdit::TextEdit*>( mTextEdit );
    sig.insertIntoTextEdit( KPIMIdentities::Signature::Start, KPIMIdentities::Signature::AddNothing,
                            pimEdit, true );
    if ( sig.type() == Signature::FromFile ) {
      setFileURL( sig.url() );
    } else {
      setFileURL( QString() );
    }

    if ( sig.type() == Signature::FromCommand ) {
      setCommandURL( sig.url() );
    } else {
      setCommandURL( QString() );
    }
  }

  void SignatureConfigurator::slotEnableEditButton( const QString & url )
  {
    mEditButton->setDisabled( url.trimmed().isEmpty() );
  }

  void SignatureConfigurator::slotEdit()
  {
    QString url = fileURL();
    // slotEnableEditButton should prevent this assert from being hit:
    assert( !url.isEmpty() );

    (void)KRun::runUrl( KUrl( url ), QString::fromLatin1( "text/plain" ), this );
  }

  QString SignatureConfigurator::asCleanedHTML() const
  {
    QString text = mTextEdit->toHtml();

    // Beautiful little hack to find the html headers produced by Qt.
    QTextDocument textDocument;
    QString html = textDocument.toHtml();

    // Now remove each line from the text, the result is clean html.
    foreach ( const QString& line, html.split( QLatin1Char('\n') ) ) {
      text.remove( line + QLatin1Char('\n') );
    }
    return text;
  }

  // "use HTML"-checkbox (un)checked
  void SignatureConfigurator::slotSetHtml()
  {
    if ( mHtmlCheck->checkState() == Qt::Unchecked ) {
      mHtmlCheck->setText( i18n( "&Use HTML" ) );
#ifndef QT_NO_TOOLBAR
      mEditToolBar->setVisible( false );
      mEditToolBar->setEnabled( false );
      mFormatToolBar->setVisible( false );
      mFormatToolBar->setEnabled( false );
#endif
      mTextEdit->switchToPlainText();
      d->inlinedHtml = false;
    }
    else {
      mHtmlCheck->setText( i18n( "&Use HTML (disabling removes formatting)" ) );
      d->inlinedHtml = true;
#ifndef QT_NO_TOOLBAR
      mEditToolBar->setVisible( true );
      mEditToolBar->setEnabled( true );
      mFormatToolBar->setVisible( true );
      mFormatToolBar->setEnabled( true );
#endif
      mTextEdit->enableRichTextMode();
    }
  }

  void SignatureConfigurator::setImageLocation ( const QString& path )
  {
    d->imageLocation = path;
  }

  void SignatureConfigurator::setImageLocation( const Identity &identity )
  {
    const QString dir = QString::fromLatin1( "emailidentities/%1/" ).arg(
        QString::number( identity.uoid() ) );
    setImageLocation( KStandardDirs::locateLocal( "data", dir ) );
  }

}

