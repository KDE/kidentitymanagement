/*  -*- c++ -*-
    SPDX-FileCopyrightText: 2008 Thomas McGuire <Thomas.McGuire@gmx.net>
    SPDX-FileCopyrightText: 2008 Edwin Schepers <yez@familieschepers.nl>
    SPDX-FileCopyrightText: 2004 Marc Mutz <mutz@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "signatureconfigurator.h"
#include "identity.h"

#include "kidentitymanagement_debug.h"
#include <KActionCollection>
#include <KIO/JobUiDelegate>
#include <KIO/OpenUrlJob>
#include <KLineEdit>
#include <KLocalizedString>
#include <KMessageBox>
#include <KShellCompletion>
#include <KToolBar>
#include <KUrlRequester>
#include <QUrl>

#include <KPIMTextEdit/RichTextComposer>
#include <KPIMTextEdit/RichTextComposerControler>
#include <KPIMTextEdit/RichTextComposerImages>

#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QFileInfo>
#include <QLabel>
#include <QStackedWidget>

#include <QHBoxLayout>
#include <QVBoxLayout>

#include <QStandardPaths>
#include <assert.h>

#include <kpimtextedit/richtexteditorwidget.h>

using namespace KIdentityManagement;

namespace KIdentityManagement
{
/**
   Private class that helps to provide binary compatibility between releases.
   @internal
  */
//@cond PRIVATE
class Q_DECL_HIDDEN SignatureConfigurator::Private
{
public:
    Private(SignatureConfigurator *parent);
    void init();
    // Returns the current text of the textedit as HTML code, but strips
    // unnecessary tags Qt inserts
    Q_REQUIRED_RESULT QString asCleanedHTML() const;

    QString imageLocation;
    SignatureConfigurator *q;
    QCheckBox *mEnableCheck = nullptr;
    QCheckBox *mHtmlCheck = nullptr;
    QComboBox *mSourceCombo = nullptr;
    KUrlRequester *mFileRequester = nullptr;
    QPushButton *mEditButton = nullptr;
    KLineEdit *mCommandEdit = nullptr;
    KToolBar *mEditToolBar = nullptr;
    KToolBar *mFormatToolBar = nullptr;
    KPIMTextEdit::RichTextComposer *mTextEdit = nullptr;
    bool inlinedHtml = false;
};
//@endcond

SignatureConfigurator::Private::Private(SignatureConfigurator *parent)
    : q(parent)
    , inlinedHtml(true)
{
}

QString SignatureConfigurator::Private::asCleanedHTML() const
{
    QString text = mTextEdit->toHtml();

    // Beautiful little hack to find the html headers produced by Qt.
    QTextDocument textDocument;
    QString html = textDocument.toHtml();

    // Now remove each line from the text, the result is clean html.
    const QStringList lst = html.split(QLatin1Char('\n'));
    for (const QString &line : lst) {
        text.remove(line + QLatin1Char('\n'));
    }
    return text;
}

void SignatureConfigurator::Private::init()
{
    auto vlay = new QVBoxLayout(q);
    vlay->setObjectName(QStringLiteral("main layout"));

    // "enable signature" checkbox:
    mEnableCheck = new QCheckBox(i18n("&Enable signature"), q);
    mEnableCheck->setWhatsThis(
        i18n("Check this box if you want KMail to append a signature to mails "
             "written with this identity."));
    vlay->addWidget(mEnableCheck);

    // "obtain signature text from" combo and label:
    auto hlay = new QHBoxLayout(); // inherits spacing
    vlay->addLayout(hlay);
    mSourceCombo = new QComboBox(q);
    mSourceCombo->setEditable(false);
    mSourceCombo->setWhatsThis(i18n("Click on the widgets below to obtain help on the input methods."));
    mSourceCombo->setEnabled(false); // since !mEnableCheck->isChecked()
    mSourceCombo->addItems(QStringList() << i18nc("continuation of \"obtain signature text from\"", "Input Field Below")
                                         << i18nc("continuation of \"obtain signature text from\"", "File")
                                         << i18nc("continuation of \"obtain signature text from\"", "Output of Command"));
    auto label = new QLabel(i18n("Obtain signature &text from:"), q);
    label->setBuddy(mSourceCombo);
    label->setEnabled(false); // since !mEnableCheck->isChecked()
    hlay->addWidget(label);
    hlay->addWidget(mSourceCombo, 1);

    // widget stack that is controlled by the source combo:
    auto widgetStack = new QStackedWidget(q);
    widgetStack->setEnabled(false); // since !mEnableCheck->isChecked()
    vlay->addWidget(widgetStack, 1);
    q->connect(mSourceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), widgetStack, &QStackedWidget::setCurrentIndex);
    q->connect(mSourceCombo, QOverload<int>::of(&QComboBox::highlighted), widgetStack, &QStackedWidget::setCurrentIndex);
    // connects for the enabling of the widgets depending on
    // signatureEnabled:
    q->connect(mEnableCheck, &QCheckBox::toggled, mSourceCombo, &QComboBox::setEnabled);
    q->connect(mEnableCheck, &QCheckBox::toggled, widgetStack, &QStackedWidget::setEnabled);
    q->connect(mEnableCheck, &QCheckBox::toggled, label, &QLabel::setEnabled);
    // The focus might be still in the widget that is disabled
    q->connect(mEnableCheck, &QCheckBox::clicked, mEnableCheck, QOverload<>::of(&QCheckBox::setFocus));

    int pageno = 0;
    // page 0: input field for direct entering:
    auto page = new QWidget(widgetStack);
    widgetStack->insertWidget(pageno, page);
    auto page_vlay = new QVBoxLayout(page);
    page_vlay->setContentsMargins(0, 0, 0, 0);

#ifndef QT_NO_TOOLBAR
    mEditToolBar = new KToolBar(q);
    mEditToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    page_vlay->addWidget(mEditToolBar, 0);

    mFormatToolBar = new KToolBar(q);
    mFormatToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    page_vlay->addWidget(mFormatToolBar, 1);
#endif

    mTextEdit = new KPIMTextEdit::RichTextComposer(q);

    auto richTextEditorwidget = new KPIMTextEdit::RichTextEditorWidget(mTextEdit, q);
    page_vlay->addWidget(richTextEditorwidget, 2);
    mTextEdit->setWhatsThis(i18n("Use this field to enter an arbitrary static signature."));

    // Fill the toolbars.
    auto actionCollection = new KActionCollection(q);
    mTextEdit->createActions(actionCollection);
#ifndef QT_NO_TOOLBAR
    mEditToolBar->addAction(actionCollection->action(QStringLiteral("format_text_bold")));
    mEditToolBar->addAction(actionCollection->action(QStringLiteral("format_text_italic")));
    mEditToolBar->addAction(actionCollection->action(QStringLiteral("format_text_underline")));
    mEditToolBar->addAction(actionCollection->action(QStringLiteral("format_text_strikeout")));
    mEditToolBar->addAction(actionCollection->action(QStringLiteral("format_text_foreground_color")));
    mEditToolBar->addAction(actionCollection->action(QStringLiteral("format_text_background_color")));
    mEditToolBar->addAction(actionCollection->action(QStringLiteral("format_font_family")));
    mEditToolBar->addAction(actionCollection->action(QStringLiteral("format_font_size")));
    mEditToolBar->addAction(actionCollection->action(QStringLiteral("format_reset")));

    mFormatToolBar->addAction(actionCollection->action(QStringLiteral("format_list_style")));
    mFormatToolBar->addAction(actionCollection->action(QStringLiteral("format_list_indent_more")));
    mFormatToolBar->addAction(actionCollection->action(QStringLiteral("format_list_indent_less")));
    mFormatToolBar->addAction(actionCollection->action(QStringLiteral("format_list_indent_less")));
    mFormatToolBar->addSeparator();

    mFormatToolBar->addAction(actionCollection->action(QStringLiteral("format_align_left")));
    mFormatToolBar->addAction(actionCollection->action(QStringLiteral("format_align_center")));
    mFormatToolBar->addAction(actionCollection->action(QStringLiteral("format_align_right")));
    mFormatToolBar->addAction(actionCollection->action(QStringLiteral("format_align_justify")));
    mFormatToolBar->addSeparator();

    mFormatToolBar->addAction(actionCollection->action(QStringLiteral("insert_horizontal_rule")));
    mFormatToolBar->addAction(actionCollection->action(QStringLiteral("manage_link")));
    mFormatToolBar->addAction(actionCollection->action(QStringLiteral("format_painter")));

    mFormatToolBar->addSeparator();
    mFormatToolBar->addAction(actionCollection->action(QStringLiteral("add_image")));
    mFormatToolBar->addSeparator();
    mFormatToolBar->addAction(actionCollection->action(QStringLiteral("insert_html")));
    mFormatToolBar->addAction(actionCollection->action(QStringLiteral("insert_table")));
#endif

    hlay = new QHBoxLayout(); // inherits spacing
    page_vlay->addLayout(hlay);
    mHtmlCheck = new QCheckBox(i18n("&Use HTML"), page);
    q->connect(mHtmlCheck, &QCheckBox::clicked, q, &SignatureConfigurator::slotSetHtml);
    hlay->addWidget(mHtmlCheck);
    inlinedHtml = true;

    widgetStack->setCurrentIndex(0); // since mSourceCombo->currentItem() == 0

    // page 1: "signature file" requester, label, "edit file" button:
    ++pageno;
    page = new QWidget(widgetStack);
    widgetStack->insertWidget(pageno, page); // force sequential numbers (play safe)
    page_vlay = new QVBoxLayout(page);
    page_vlay->setContentsMargins(0, 0, 0, 0);
    hlay = new QHBoxLayout(); // inherits spacing
    page_vlay->addLayout(hlay);
    mFileRequester = new KUrlRequester(page);
    mFileRequester->setFilter(QStringLiteral("*.txt"));
    mFileRequester->setWhatsThis(
        i18n("Use this requester to specify a text file that contains your "
             "signature. It will be read every time you create a new mail or "
             "append a new signature."));
    label = new QLabel(i18n("S&pecify file:"), page);
    label->setBuddy(mFileRequester);
    hlay->addWidget(label);
    hlay->addWidget(mFileRequester, 1);
    mFileRequester->button()->setAutoDefault(false);
    q->connect(mFileRequester, &KUrlRequester::textEdited, q, &SignatureConfigurator::slotUrlChanged);
    q->connect(mFileRequester, &KUrlRequester::urlSelected, q, &SignatureConfigurator::slotUrlChanged);
    mEditButton = new QPushButton(i18n("Edit &File"), page);
    mEditButton->setWhatsThis(i18n("Opens the specified file in a text editor."));
    q->connect(mEditButton, &QPushButton::clicked, q, &SignatureConfigurator::slotEdit);
    mEditButton->setAutoDefault(false);
    mEditButton->setEnabled(false); // initially nothing to edit
    hlay->addWidget(mEditButton);
    page_vlay->addStretch(1); // spacer

    // page 2: "signature command" requester and label:
    ++pageno;
    page = new QWidget(widgetStack);
    widgetStack->insertWidget(pageno, page);
    page_vlay = new QVBoxLayout(page);
    page_vlay->setContentsMargins(0, 0, 0, 0);
    hlay = new QHBoxLayout(); // inherits spacing
    page_vlay->addLayout(hlay);
    mCommandEdit = new KLineEdit(page);
    mCommandEdit->setClearButtonEnabled(true);
    mCommandEdit->setCompletionObject(new KShellCompletion());
    mCommandEdit->setAutoDeleteCompletionObject(true);
    mCommandEdit->setWhatsThis(
        i18n("You can add an arbitrary command here, either with or without path "
             "depending on whether or not the command is in your Path. For every "
             "new mail, KMail will execute the command and use what it outputs (to "
             "standard output) as a signature. Usual commands for use with this "
             "mechanism are \"fortune\" or \"ksig -random\". "
             "(Be careful, script needs a shebang line)."));
    label = new QLabel(i18n("S&pecify command:"), page);
    label->setBuddy(mCommandEdit);
    hlay->addWidget(label);
    hlay->addWidget(mCommandEdit, 1);
    page_vlay->addStretch(1); // spacer
}

SignatureConfigurator::SignatureConfigurator(QWidget *parent)
    : QWidget(parent)
    , d(new Private(this))
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

void SignatureConfigurator::setSignatureEnabled(bool enable)
{
    d->mEnableCheck->setChecked(enable);
}

Signature::Type SignatureConfigurator::signatureType() const
{
    switch (d->mSourceCombo->currentIndex()) {
    case 0:
        return Signature::Inlined;
    case 1:
        return Signature::FromFile;
    case 2:
        return Signature::FromCommand;
    default:
        return Signature::Disabled;
    }
}

void SignatureConfigurator::setSignatureType(Signature::Type type)
{
    int idx = 0;
    switch (type) {
    case Signature::Inlined:
        idx = 0;
        break;
    case Signature::FromFile:
        idx = 1;
        break;
    case Signature::FromCommand:
        idx = 2;
        break;
    default:
        idx = 0;
        break;
    }

    d->mSourceCombo->setCurrentIndex(idx);
}

void SignatureConfigurator::setInlineText(const QString &text)
{
    d->mTextEdit->setTextOrHtml(text);
}

QString SignatureConfigurator::filePath() const
{
    QString file = d->mFileRequester->url().path();

    // Force the filename to be relative to ~ instead of $PWD depending
    // on the rest of the code (KRun::run in Edit and KFileItem on save)
    if (!file.isEmpty() && QFileInfo(file).isRelative()) {
        file = QDir::home().absolutePath() + QLatin1Char('/') + file;
    }
    return file;
}

void SignatureConfigurator::setFileURL(const QString &url)
{
    d->mFileRequester->setUrl(QUrl::fromLocalFile(url));
    d->mEditButton->setDisabled(url.trimmed().isEmpty());
}

QString SignatureConfigurator::commandPath() const
{
    return d->mCommandEdit->text();
}

void SignatureConfigurator::setCommandURL(const QString &url)
{
    d->mCommandEdit->setText(url);
}

Signature SignatureConfigurator::signature() const
{
    Signature sig;
    const Signature::Type sigType = signatureType();
    switch (sigType) {
    case Signature::Inlined:
        sig.setInlinedHtml(d->inlinedHtml);
        sig.setText(d->inlinedHtml ? d->asCleanedHTML() : d->mTextEdit->textOrHtml());
        if (d->inlinedHtml) {
            if (!d->imageLocation.isEmpty()) {
                sig.setImageLocation(d->imageLocation);
            }
            const KPIMTextEdit::ImageWithNameList images = d->mTextEdit->composerControler()->composerImages()->imagesWithName();
            for (const KPIMTextEdit::ImageWithNamePtr &image : images) {
                sig.addImage(image->image, image->name);
            }
        }
        break;
    case Signature::FromCommand:
        sig.setPath(commandPath(), true);
        break;
    case Signature::FromFile:
        sig.setPath(filePath(), false);
        break;
    case Signature::Disabled:
        /* do nothing */
        break;
    }
    sig.setEnabledSignature(isSignatureEnabled());
    sig.setType(sigType);
    return sig;
}

void SignatureConfigurator::setSignature(const Signature &sig)
{
    setSignatureType(sig.type());
    setSignatureEnabled(sig.isEnabledSignature());

    if (sig.isInlinedHtml()) {
        d->mHtmlCheck->setCheckState(Qt::Checked);
    } else {
        d->mHtmlCheck->setCheckState(Qt::Unchecked);
    }
    slotSetHtml();

    // Let insertIntoTextEdit() handle setting the text, as that function also adds the images.
    d->mTextEdit->clear();
    sig.insertIntoTextEdit(KIdentityManagement::Signature::Start, KIdentityManagement::Signature::AddNothing, d->mTextEdit, true);
    if (sig.type() == Signature::FromFile) {
        setFileURL(sig.path());
    } else {
        setFileURL(QString());
    }

    if (sig.type() == Signature::FromCommand) {
        setCommandURL(sig.path());
    } else {
        setCommandURL(QString());
    }
}

void SignatureConfigurator::slotUrlChanged()
{
    const QString file = filePath();
    const QFileInfo infoFile(file);
    if (infoFile.isFile() && (infoFile.size() > 1000)) {
        KMessageBox::information(this, i18n("This text file size exceeds 1kb."), i18n("Text File Size"));
    }
    d->mEditButton->setDisabled(file.isEmpty());
}

void SignatureConfigurator::slotEdit()
{
    const QString url = filePath();
    // slotEnableEditButton should prevent this assert from being hit:
    assert(!url.isEmpty());

    auto job = new KIO::OpenUrlJob(QUrl::fromLocalFile(url), QStringLiteral("text/plain"));
    job->setUiDelegate(new KIO::JobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, this));
    job->setDeleteTemporaryFile(true);
    job->start();
}

// "use HTML"-checkbox (un)checked
void SignatureConfigurator::slotSetHtml()
{
    if (d->mHtmlCheck->checkState() == Qt::Unchecked) {
        d->mHtmlCheck->setText(i18n("&Use HTML"));
#ifndef QT_NO_TOOLBAR
        d->mEditToolBar->setVisible(false);
        d->mEditToolBar->setEnabled(false);
        d->mFormatToolBar->setVisible(false);
        d->mFormatToolBar->setEnabled(false);
#endif
        d->mTextEdit->switchToPlainText();
        d->inlinedHtml = false;
    } else {
        d->mHtmlCheck->setText(i18n("&Use HTML (disabling removes formatting)"));
        d->inlinedHtml = true;
#ifndef QT_NO_TOOLBAR
        d->mEditToolBar->setVisible(true);
        d->mEditToolBar->setEnabled(true);
        d->mFormatToolBar->setVisible(true);
        d->mFormatToolBar->setEnabled(true);
#endif
        d->mTextEdit->activateRichText();
    }
}

void SignatureConfigurator::setImageLocation(const QString &path)
{
    d->imageLocation = path;
}

void SignatureConfigurator::setImageLocation(const Identity &identity)
{
    const QString dir =
        QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/emailidentities/%1/").arg(QString::number(identity.uoid()));
    QDir().mkpath(dir);
    setImageLocation(dir);
}
}
