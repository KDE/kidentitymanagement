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
#include <qdebug.h>
#include <kdialog.h>
#include <klineedit.h>
#include <kurlrequester.h>
#include <kshellcompletion.h>
#include <ktoolbar.h>
#include <krun.h>
#include <QUrl>
#include <KComboBox>

#include <kpimtextedit/textedit.h>

#include <QCheckBox>
#include <QDir>
#include <QFileInfo>
#include <QLabel>

#include <QStackedWidget>

#include <QVBoxLayout>
#include <QHBoxLayout>

#include <assert.h>
#include <QStandardPaths>

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
    // Returns the current text of the textedit as HTML code, but strips
    // unnecessary tags Qt inserts
    QString asCleanedHTML() const;

    SignatureConfigurator *q;
    bool inlinedHtml;
    QString imageLocation;
    QCheckBox       * mEnableCheck;
    QCheckBox       * mHtmlCheck;
    KComboBox       * mSourceCombo;
    KUrlRequester   * mFileRequester;
    QPushButton     * mEditButton;
    KLineEdit       * mCommandEdit;
    KToolBar        * mEditToolBar;
    KToolBar        * mFormatToolBar;
    KRichTextWidget * mTextEdit;      // Grmbl, why is this not in the private class? 
                                      // This is a KPIMTextEdit::TextEdit, really.

};
//@endcond

SignatureConfigurator::Private::Private( SignatureConfigurator *parent )
  :q( parent ), inlinedHtml( true )
{
}

QString SignatureConfigurator::Private::asCleanedHTML() const
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

void SignatureConfigurator::Private::init()
{
  // tmp. vars:
  QLabel * label;
  QWidget * page;
  QHBoxLayout * hlay;
  QVBoxLayout * vlay;
  QVBoxLayout * page_vlay;

  vlay = new QVBoxLayout( q );
  vlay->setObjectName( QStringLiteral("main layout") );
  vlay->setMargin( 0 );

  // "enable signatue" checkbox:
  mEnableCheck = new QCheckBox( i18n( "&Enable signature" ), q );
  mEnableCheck->setWhatsThis(
      i18n( "Check this box if you want KMail to append a signature to mails "
            "written with this identity." ) );
  vlay->addWidget( mEnableCheck );

  // "obtain signature text from" combo and label:
  hlay = new QHBoxLayout(); // inherits spacing
  vlay->addLayout( hlay );
  mSourceCombo = new KComboBox( q );
  mSourceCombo->setEditable( false );
  mSourceCombo->setWhatsThis(
      i18n( "Click on the widgets below to obtain help on the input methods." ) );
  mSourceCombo->setEnabled( false ); // since !mEnableCheck->isChecked()
  mSourceCombo->addItems( QStringList()
                  << i18nc( "continuation of \"obtain signature text from\"",
                            "Input Field Below" )
                  << i18nc( "continuation of \"obtain signature text from\"",
                            "File" )
                  << i18nc( "continuation of \"obtain signature text from\"",
                            "Output of Command" ) );
  label = new QLabel( i18n( "Obtain signature &text from:" ), q );
  label->setBuddy( mSourceCombo );
  label->setEnabled( false ); // since !mEnableCheck->isChecked()
  hlay->addWidget( label );
  hlay->addWidget( mSourceCombo, 1 );

  // widget stack that is controlled by the source combo:
  QStackedWidget * widgetStack = new QStackedWidget( q );
  widgetStack->setEnabled( false ); // since !mEnableCheck->isChecked()
  vlay->addWidget( widgetStack, 1 );
  q->connect( mSourceCombo, SIGNAL(currentIndexChanged(int)),
              widgetStack, SLOT(setCurrentIndex(int)) );
  q->connect( mSourceCombo, SIGNAL(highlighted(int)),
              widgetStack, SLOT(setCurrentIndex(int)) );
  // connects for the enabling of the widgets depending on
  // signatureEnabled:
  q->connect( mEnableCheck, SIGNAL(toggled(bool)),
              mSourceCombo, SLOT(setEnabled(bool)) );
  q->connect( mEnableCheck, SIGNAL(toggled(bool)),
              widgetStack, SLOT(setEnabled(bool)) );
  q->connect( mEnableCheck, SIGNAL(toggled(bool)),
              label, SLOT(setEnabled(bool)) );
  // The focus might be still in the widget that is disabled
  q->connect( mEnableCheck, SIGNAL(clicked()),
              mEnableCheck, SLOT(setFocus()) );

  int pageno = 0;
  // page 0: input field for direct entering:
  page = new QWidget( widgetStack );
  widgetStack->insertWidget( pageno, page );
  page_vlay = new QVBoxLayout( page );

#ifndef QT_NO_TOOLBAR
  mEditToolBar = new KToolBar( q );
  mEditToolBar->setToolButtonStyle( Qt::ToolButtonIconOnly );
  page_vlay->addWidget( mEditToolBar, 0 );

  mFormatToolBar = new KToolBar( q );
  mFormatToolBar->setToolButtonStyle( Qt::ToolButtonIconOnly );
  page_vlay->addWidget( mFormatToolBar, 1 );
#endif

  mTextEdit = new KPIMTextEdit::TextEdit( q );
  static_cast<KPIMTextEdit::TextEdit*>( mTextEdit )->enableImageActions();
  static_cast<KPIMTextEdit::TextEdit*>( mTextEdit )->enableInsertHtmlActions();
  static_cast<KPIMTextEdit::TextEdit*>( mTextEdit )->enableInsertTableActions();
  page_vlay->addWidget( mTextEdit, 2 );
  mTextEdit->setWhatsThis( i18n( "Use this field to enter an arbitrary static signature." ) );
  // exclude SupportToPlainText.
  mTextEdit->setRichTextSupport( KRichTextWidget::FullTextFormattingSupport |
      KRichTextWidget::FullListSupport |
      KRichTextWidget::SupportAlignment |
      KRichTextWidget::SupportRuleLine |
      KRichTextWidget::SupportHyperlinks |
      KRichTextWidget::SupportFormatPainting );

  // Fill the toolbars.
  KActionCollection *actionCollection = new KActionCollection( q );
  actionCollection->addActions(mTextEdit->createActions());
#ifndef QT_NO_TOOLBAR
  mEditToolBar->addAction( actionCollection->action( QStringLiteral("format_text_bold") ) );
  mEditToolBar->addAction( actionCollection->action( QStringLiteral("format_text_italic") ) );
  mEditToolBar->addAction( actionCollection->action( QStringLiteral("format_text_underline") ) );
  mEditToolBar->addAction( actionCollection->action( QStringLiteral("format_text_strikeout") ) );
  mEditToolBar->addAction( actionCollection->action( QStringLiteral("format_text_foreground_color") ) );
  mEditToolBar->addAction( actionCollection->action( QStringLiteral("format_text_background_color") ) );
  mEditToolBar->addAction( actionCollection->action( QStringLiteral("format_font_family") ) );
  mEditToolBar->addAction( actionCollection->action( QStringLiteral("format_font_size") ) );
  mEditToolBar->addAction( actionCollection->action( QStringLiteral("format_reset") ) );

  mFormatToolBar->addAction( actionCollection->action( QStringLiteral("format_list_style") ) );
  mFormatToolBar->addAction( actionCollection->action( QStringLiteral("format_list_indent_more") ) );
  mFormatToolBar->addAction( actionCollection->action( QStringLiteral("format_list_indent_less") ) );
  mFormatToolBar->addAction( actionCollection->action( QStringLiteral("format_list_indent_less") ) );
  mFormatToolBar->addSeparator();

  mFormatToolBar->addAction( actionCollection->action( QStringLiteral("format_align_left") ) );
  mFormatToolBar->addAction( actionCollection->action( QStringLiteral("format_align_center") ) );
  mFormatToolBar->addAction( actionCollection->action( QStringLiteral("format_align_right") ) );
  mFormatToolBar->addAction( actionCollection->action( QStringLiteral("format_align_justify") ) );
  mFormatToolBar->addSeparator();

  mFormatToolBar->addAction( actionCollection->action( QStringLiteral("insert_horizontal_rule") ) );
  mFormatToolBar->addAction( actionCollection->action( QStringLiteral("manage_link") ) );
  mFormatToolBar->addAction( actionCollection->action( QStringLiteral("format_painter") ) );

  mFormatToolBar->addSeparator();
  mFormatToolBar->addAction( actionCollection->action( QStringLiteral("add_image") ) );
  mFormatToolBar->addSeparator();
  mFormatToolBar->addAction( actionCollection->action( QStringLiteral("insert_html") ) );
  mFormatToolBar->addAction( actionCollection->action( QStringLiteral("insert_table" )) );
#endif

  hlay = new QHBoxLayout(); // inherits spacing
  page_vlay->addLayout( hlay );
  mHtmlCheck = new QCheckBox( i18n( "&Use HTML" ), page );
  q->connect( mHtmlCheck, SIGNAL(clicked()),
              q, SLOT(slotSetHtml()) );
  hlay->addWidget( mHtmlCheck );
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
  mFileRequester = new KUrlRequester( page );
  mFileRequester->setWhatsThis(
      i18n( "Use this requester to specify a text file that contains your "
            "signature. It will be read every time you create a new mail or "
            "append a new signature." ) );
  label = new QLabel( i18n( "S&pecify file:" ), page );
  label->setBuddy( mFileRequester );
  hlay->addWidget( label );
  hlay->addWidget( mFileRequester, 1 );
  mFileRequester->button()->setAutoDefault( false );
  q->connect( mFileRequester, SIGNAL(textChanged(QString)),
              q, SLOT(slotEnableEditButton(QString)) );
  mEditButton = new QPushButton( i18n( "Edit &File" ), page );
  mEditButton->setWhatsThis( i18n( "Opens the specified file in a text editor." ) );
  q->connect( mEditButton, SIGNAL(clicked()),
              q, SLOT(slotEdit()) );
  mEditButton->setAutoDefault( false );
  mEditButton->setEnabled( false ); // initially nothing to edit
  hlay->addWidget( mEditButton );
  page_vlay->addStretch( 1 ); // spacer

  // page 2: "signature command" requester and label:
  ++pageno;
  page = new QWidget( widgetStack );
  widgetStack->insertWidget( pageno, page );
  page_vlay = new QVBoxLayout( page );
  page_vlay->setMargin( 0 );
  hlay = new QHBoxLayout(); // inherits spacing
  page_vlay->addLayout( hlay );
  mCommandEdit = new KLineEdit( page );
  mCommandEdit->setClearButtonShown( true );
  mCommandEdit->setCompletionObject( new KShellCompletion() );
  mCommandEdit->setAutoDeleteCompletionObject( true );
  mCommandEdit->setWhatsThis(
      i18n( "You can add an arbitrary command here, either with or without path "
            "depending on whether or not the command is in your Path. For every "
            "new mail, KMail will execute the command and use what it outputs (to "
            "standard output) as a signature. Usual commands for use with this "
            "mechanism are \"fortune\" or \"ksig -random\"." ) );
  label = new QLabel( i18n( "S&pecify command:" ), page );
  label->setBuddy( mCommandEdit );
  hlay->addWidget( label );
  hlay->addWidget( mCommandEdit, 1 );
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
    return d->mEnableCheck->isChecked();
  }

  void SignatureConfigurator::setSignatureEnabled( bool enable )
  {
    d->mEnableCheck->setChecked( enable );
  }

  Signature::Type SignatureConfigurator::signatureType() const
  {
    switch ( d->mSourceCombo->currentIndex() ) {
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

    d->mSourceCombo->setCurrentIndex( idx );
  }

  void SignatureConfigurator::setInlineText( const QString & text )
  {
    d->mTextEdit->setTextOrHtml( text );
  }

  QString SignatureConfigurator::fileURL() const
  {
    QString file = d->mFileRequester->url().path();

    // Force the filename to be relative to ~ instead of $PWD depending
    // on the rest of the code (KRun::run in Edit and KFileItem on save)
    if ( !file.isEmpty() && QFileInfo( file ).isRelative() ) {
      file = QDir::home().absolutePath() + QDir::separator() + file;
    }
    return file;
  }

  void SignatureConfigurator::setFileURL( const QString & url )
  {
    d->mFileRequester->setUrl( QUrl(url) );
  }

  QString SignatureConfigurator::commandURL() const
  {
    return d->mCommandEdit->text();
  }

  void SignatureConfigurator::setCommandURL( const QString & url )
  {
    d->mCommandEdit->setText( url );
  }


  Signature SignatureConfigurator::signature() const
  {
    Signature sig;
    const Signature::Type sigType = signatureType();
    switch ( sigType ) {
    case Signature::Inlined:
      sig.setInlinedHtml( d->inlinedHtml );
      sig.setText( d->inlinedHtml ? d->asCleanedHTML() : d->mTextEdit->textOrHtml() );
      if ( d->inlinedHtml ) {
        if ( !d->imageLocation.isEmpty() ) {
          sig.setImageLocation( d->imageLocation );
        }
        KPIMTextEdit::ImageWithNameList images = static_cast< KPIMTextEdit::TextEdit*>( d->mTextEdit )->imagesWithName();
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
      d->mHtmlCheck->setCheckState( Qt::Checked );
    } else {
      d->mHtmlCheck->setCheckState( Qt::Unchecked );
    }
    slotSetHtml();

    // Let insertIntoTextEdit() handle setting the text, as that function also adds the images.
    d->mTextEdit->clear();
    KPIMTextEdit::TextEdit * const pimEdit = static_cast<KPIMTextEdit::TextEdit*>( d->mTextEdit );
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
    d->mEditButton->setDisabled( url.trimmed().isEmpty() );
  }

  void SignatureConfigurator::slotEdit()
  {
    QString url = fileURL();
    // slotEnableEditButton should prevent this assert from being hit:
    assert( !url.isEmpty() );

    (void)KRun::runUrl( QUrl( url ), QString::fromLatin1( "text/plain" ), this );
  }

  // "use HTML"-checkbox (un)checked
  void SignatureConfigurator::slotSetHtml()
  {
    if ( d->mHtmlCheck->checkState() == Qt::Unchecked ) {
      d->mHtmlCheck->setText( i18n( "&Use HTML" ) );
#ifndef QT_NO_TOOLBAR
      d->mEditToolBar->setVisible( false );
      d->mEditToolBar->setEnabled( false );
      d->mFormatToolBar->setVisible( false );
      d->mFormatToolBar->setEnabled( false );
#endif
      d->mTextEdit->switchToPlainText();
      d->inlinedHtml = false;
    }
    else {
      d->mHtmlCheck->setText( i18n( "&Use HTML (disabling removes formatting)" ) );
      d->inlinedHtml = true;
#ifndef QT_NO_TOOLBAR
      d->mEditToolBar->setVisible( true );
      d->mEditToolBar->setEnabled( true );
      d->mFormatToolBar->setVisible( true );
      d->mFormatToolBar->setEnabled( true );
#endif
      d->mTextEdit->enableRichTextMode();
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
    setImageLocation( QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + dir ) ;
  }

}

