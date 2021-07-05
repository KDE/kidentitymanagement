/*
    SPDX-FileCopyrightText: 2002-2004 Marc Mutz <mutz@kde.org>
    SPDX-FileCopyrightText: 2007 Tom Albers <tomalbers@kde.nl>
    SPDX-FileCopyrightText: 2009 Thomas McGuire <mcguire@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "signature.h"

#include "kidentitymanagement_debug.h"
#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>
#include <KProcess>

#include <QSharedPointer>

#include <QDir>
#include <assert.h>
#include <kpimtextedit/richtextcomposer.h>
#include <kpimtextedit/richtextcomposercontroler.h>
#include <kpimtextedit/richtextcomposerimages.h>

using namespace KIdentityManagement;

class Q_DECL_HIDDEN KIdentityManagement::SignaturePrivate
{
public:
    SignaturePrivate(Signature *qq)
        : q(qq)
    {
    }

    void assignFrom(const KIdentityManagement::Signature &that);
    void cleanupImages();
    void saveImages() const;
    Q_REQUIRED_RESULT QString textFromFile(bool *ok) const;
    Q_REQUIRED_RESULT QString textFromCommand(bool *ok) const;
    void insertSignatureText(Signature::Placement placement, Signature::AddedText addedText, KPIMTextEdit::RichTextComposer *textEdit, bool forceDisplay) const;

    /// List of images that belong to this signature. Either added by addImage() or
    /// by readConfig().
    QVector<Signature::EmbeddedImagePtr> embeddedImages;

    /// The directory where the images will be saved to.
    QString saveLocation;
    QString path;
    QString text;
    Signature::Type type = Signature::Disabled;
    bool enabled = false;
    bool inlinedHtml = false;
    Signature *const q;
};

static bool isCursorAtEndOfLine(const QTextCursor &cursor)
{
    QTextCursor testCursor = cursor;
    testCursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    return !testCursor.hasSelection();
}

static void
insertSignatureHelper(const QString &signature, KPIMTextEdit::RichTextComposer *textEdit, Signature::Placement placement, bool isHtml, bool addNewlines)
{
    if (!signature.isEmpty()) {
        // Save the modified state of the document, as inserting a signature
        // shouldn't change this. Restore it at the end of this function.
        bool isModified = textEdit->document()->isModified();

        // Move to the desired position, where the signature should be inserted
        QTextCursor cursor = textEdit->textCursor();
        QTextCursor oldCursor = cursor;
        cursor.beginEditBlock();

        if (placement == Signature::End) {
            cursor.movePosition(QTextCursor::End);
        } else if (placement == Signature::Start) {
            cursor.movePosition(QTextCursor::Start);
        } else if (placement == Signature::AtCursor) {
            cursor.movePosition(QTextCursor::StartOfLine);
        }
        textEdit->setTextCursor(cursor);

        QString lineSep;
        if (addNewlines) {
            if (isHtml) {
                lineSep = QStringLiteral("<br>");
            } else {
                lineSep = QLatin1Char('\n');
            }
        }

        // Insert the signature and newlines depending on where it was inserted.
        int newCursorPos = -1;
        QString headSep;
        QString tailSep;

        if (placement == Signature::End) {
            // There is one special case when re-setting the old cursor: The cursor
            // was at the end. In this case, QTextEdit has no way to know
            // if the signature was added before or after the cursor, and just
            // decides that it was added before (and the cursor moves to the end,
            // but it should not when appending a signature). See bug 167961
            if (oldCursor.position() == textEdit->toPlainText().length()) {
                newCursorPos = oldCursor.position();
            }
            headSep = lineSep;
        } else if (placement == Signature::Start) {
            // When prepending signatures, add a couple of new lines before
            // the signature, and move the cursor to the beginning of the QTextEdit.
            // People tends to insert new text there.
            newCursorPos = 0;
            headSep = lineSep + lineSep;
            if (!isCursorAtEndOfLine(cursor)) {
                tailSep = lineSep;
            }
        } else if (placement == Signature::AtCursor) {
            if (!isCursorAtEndOfLine(cursor)) {
                tailSep = lineSep;
            }
        }

        const QString full_signature = headSep + signature + tailSep;
        if (isHtml) {
            textEdit->insertHtml(full_signature);
        } else {
            textEdit->insertPlainText(full_signature);
        }

        cursor.endEditBlock();
        if (newCursorPos != -1) {
            oldCursor.setPosition(newCursorPos);
        }

        textEdit->setTextCursor(oldCursor);
        textEdit->ensureCursorVisible();

        textEdit->document()->setModified(isModified);

        if (isHtml) {
            textEdit->activateRichText();
        }
    }
}

// Returns the names of all images in the HTML code
static QStringList findImageNames(const QString &htmlCode)
{
    QStringList ret;

    // To complicated for us, so cheat and let a text edit do the hard work
    KPIMTextEdit::RichTextComposer edit;
    edit.setHtml(htmlCode);
    const KPIMTextEdit::ImageWithNameList images = edit.composerControler()->composerImages()->imagesWithName();
    ret.reserve(images.count());
    for (const KPIMTextEdit::ImageWithNamePtr &image : images) {
        ret << image->name;
    }
    return ret;
}

void SignaturePrivate::assignFrom(const KIdentityManagement::Signature &that)
{
    path = that.path();
    inlinedHtml = that.isInlinedHtml();
    text = that.text();
    type = that.type();
    enabled = that.isEnabledSignature();
    saveLocation = that.imageLocation();
    embeddedImages = that.embeddedImages();
}

void SignaturePrivate::cleanupImages()
{
    // Remove any images from the internal structure that are no longer there
    if (inlinedHtml) {
        auto it = std::remove_if(embeddedImages.begin(), embeddedImages.end(), [this](const Signature::EmbeddedImagePtr &imageInList) {
            const QStringList lstImage = findImageNames(text);
            for (const QString &imageInHtml : lstImage) {
                if (imageInHtml == imageInList->name) {
                    return false;
                }
            }
            return true;
        });
        embeddedImages.erase(it, embeddedImages.end());
    }

    // Delete all the old image files
    if (!saveLocation.isEmpty()) {
        QDir dir(saveLocation);
        const QStringList lst = dir.entryList(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
        for (const QString &fileName : lst) {
            if (fileName.endsWith(QLatin1String(".png"), Qt::CaseInsensitive)) {
                qCDebug(KIDENTITYMANAGEMENT_LOG) << "Deleting old image" << dir.path() + fileName;
                dir.remove(fileName);
            }
        }
    }
}

void SignaturePrivate::saveImages() const
{
    if (inlinedHtml && !saveLocation.isEmpty()) {
        for (const Signature::EmbeddedImagePtr &image : std::as_const(embeddedImages)) {
            const QString location = saveLocation + QLatin1Char('/') + image->name;
            if (!image->image.save(location, "PNG")) {
                qCWarning(KIDENTITYMANAGEMENT_LOG) << "Failed to save image" << location;
            }
        }
    }
}

QString SignaturePrivate::textFromFile(bool *ok) const
{
    assert(type == Signature::FromFile);

    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        qCWarning(KIDENTITYMANAGEMENT_LOG) << "Failed to open" << path << ":" << f.errorString();
        if (ok) {
            *ok = false;
        }
        return QString();
    }

    if (ok) {
        *ok = true;
    }
    const QByteArray ba = f.readAll();
    return QString::fromLocal8Bit(ba.data(), ba.size());
}

QString SignaturePrivate::textFromCommand(bool *ok) const
{
    assert(type == Signature::FromCommand);

    // handle pathological cases:
    if (path.isEmpty()) {
        if (ok) {
            *ok = true;
        }
        return QString();
    }

    // create a shell process:
    KProcess proc;
    proc.setOutputChannelMode(KProcess::SeparateChannels);
    proc.setShellCommand(path);
    int rc = proc.execute();

    // handle errors, if any:
    if (rc != 0) {
        if (ok) {
            *ok = false;
        }
        const QString wmsg = i18n(
            "<qt>Failed to execute signature script<p><b>%1</b>:</p>"
            "<p>%2</p></qt>",
            path,
            QString::fromUtf8(proc.readAllStandardError()));
        KMessageBox::error(nullptr, wmsg);
        return QString();
    }

    // no errors:
    if (ok) {
        *ok = true;
    }

    // get output:
    const QByteArray output = proc.readAllStandardOutput();

    // TODO: hmm, should we allow other encodings, too?
    return QString::fromLocal8Bit(output.data(), output.size());
}

void SignaturePrivate::insertSignatureText(Signature::Placement placement,
                                           Signature::AddedText addedText,
                                           KPIMTextEdit::RichTextComposer *textEdit,
                                           bool forceDisplay) const
{
    if (!forceDisplay) {
        if (!enabled) {
            return;
        }
    }
    QString signature;
    if (addedText & Signature::AddSeparator) {
        signature = q->withSeparator();
    } else {
        signature = q->rawText();
    }
    insertSignatureHelper(signature,
                          textEdit,
                          placement,
                          (inlinedHtml && type == KIdentityManagement::Signature::Inlined),
                          (addedText & Signature::AddNewLines));

    // We added the text of the signature above, now it is time to add the images as well.
    if (inlinedHtml) {
        for (const Signature::EmbeddedImagePtr &image : std::as_const(embeddedImages)) {
            textEdit->composerControler()->composerImages()->loadImage(image->image, image->name, image->name);
        }
    }
}

QDataStream &operator<<(QDataStream &stream, const KIdentityManagement::Signature::EmbeddedImagePtr &img)
{
    return stream << img->image << img->name;
}

QDataStream &operator>>(QDataStream &stream, KIdentityManagement::Signature::EmbeddedImagePtr &img)
{
    return stream >> img->image >> img->name;
}

Signature::Signature()
    : d(new SignaturePrivate(this))
{
    d->type = Disabled;
    d->inlinedHtml = false;
}

Signature::Signature(const QString &text)
    : d(new SignaturePrivate(this))
{
    d->type = Inlined;
    d->inlinedHtml = false;
    d->text = text;
}

Signature::Signature(const QString &path, bool isExecutable)
    : d(new SignaturePrivate(this))
{
    d->type = isExecutable ? FromCommand : FromFile;
    d->path = path;
}

Signature::Signature(const Signature &that)
    : d(new SignaturePrivate(this))
{
    d->assignFrom(that);
}

Signature &Signature::operator=(const KIdentityManagement::Signature &that)
{
    if (this == &that) {
        return *this;
    }

    d->assignFrom(that);
    return *this;
}

Signature::~Signature()
{
    delete d;
}

QString Signature::rawText(bool *ok) const
{
    switch (d->type) {
    case Disabled:
        if (ok) {
            *ok = true;
        }
        return QString();
    case Inlined:
        if (ok) {
            *ok = true;
        }
        return d->text;
    case FromFile:
        return d->textFromFile(ok);
    case FromCommand:
        return d->textFromCommand(ok);
    }
    qCritical() << "Signature::type() returned unknown value!";
    return QString(); // make compiler happy
}

QString Signature::withSeparator(bool *ok) const
{
    QString signature = rawText(ok);
    if (ok && (*ok) == false) {
        return QString();
    }

    if (signature.isEmpty()) {
        return signature; // don't add a separator in this case
    }

    const bool htmlSig = (isInlinedHtml() && d->type == Inlined);
    QString newline = htmlSig ? QStringLiteral("<br>") : QStringLiteral("\n");
    if (htmlSig && signature.startsWith(QLatin1String("<p"))) {
        newline.clear();
    }

    if (signature.startsWith(QLatin1String("-- ") + newline) || (signature.indexOf(newline + QLatin1String("-- ") + newline) != -1)) {
        // already have signature separator at start of sig or inside sig:
        return signature;
    } else {
        // need to prepend one:
        return QLatin1String("-- ") + newline + signature;
    }
}

void Signature::setPath(const QString &path, bool isExecutable)
{
    d->path = path;
    d->type = isExecutable ? FromCommand : FromFile;
}

void Signature::setInlinedHtml(bool isHtml)
{
    d->inlinedHtml = isHtml;
}

bool Signature::isInlinedHtml() const
{
    return d->inlinedHtml;
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
static const char sigEnabled[] = "Signature Enabled";

void Signature::readConfig(const KConfigGroup &config)
{
    QString sigType = config.readEntry(sigTypeKey);
    if (sigType == QLatin1String(sigTypeInlineValue)) {
        d->type = Inlined;
        d->inlinedHtml = config.readEntry(sigTypeInlinedHtmlKey, false);
    } else if (sigType == QLatin1String(sigTypeFileValue)) {
        d->type = FromFile;
        d->path = config.readPathEntry(sigFileKey, QString());
    } else if (sigType == QLatin1String(sigTypeCommandValue)) {
        d->type = FromCommand;
        d->path = config.readPathEntry(sigCommandKey, QString());
    } else if (sigType == QLatin1String(sigTypeDisabledValue)) {
        d->enabled = false;
    }
    if (d->type != Disabled) {
        d->enabled = config.readEntry(sigEnabled, true);
    }

    d->text = config.readEntry(sigTextKey);
    d->saveLocation = config.readEntry(sigImageLocation);

    if (isInlinedHtml() && !d->saveLocation.isEmpty()) {
        QDir dir(d->saveLocation);
        const QStringList lst = dir.entryList(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
        for (const QString &fileName : lst) {
            if (fileName.endsWith(QLatin1String(".png"), Qt::CaseInsensitive)) {
                QImage image;
                if (image.load(dir.path() + QLatin1Char('/') + fileName)) {
                    addImage(image, fileName);
                } else {
                    qCWarning(KIDENTITYMANAGEMENT_LOG) << "Unable to load image" << dir.path() + QLatin1Char('/') + fileName;
                }
            }
        }
    }
}

void Signature::writeConfig(KConfigGroup &config) const
{
    switch (d->type) {
    case Inlined:
        config.writeEntry(sigTypeKey, sigTypeInlineValue);
        config.writeEntry(sigTypeInlinedHtmlKey, d->inlinedHtml);
        break;
    case FromFile:
        config.writeEntry(sigTypeKey, sigTypeFileValue);
        config.writePathEntry(sigFileKey, d->path);
        break;
    case FromCommand:
        config.writeEntry(sigTypeKey, sigTypeCommandValue);
        config.writePathEntry(sigCommandKey, d->path);
        break;
    default:
        break;
    }
    config.writeEntry(sigTextKey, d->text);
    config.writeEntry(sigImageLocation, d->saveLocation);
    config.writeEntry(sigEnabled, d->enabled);

    d->cleanupImages();
    d->saveImages();
}

void Signature::insertIntoTextEdit(Placement placement, AddedText addedText, KPIMTextEdit::RichTextComposer *textEdit, bool forceDisplay) const
{
    d->insertSignatureText(placement, addedText, textEdit, forceDisplay);
}

QVector<Signature::EmbeddedImagePtr> Signature::embeddedImages() const
{
    return d->embeddedImages;
}

void Signature::setEmbeddedImages(const QVector<Signature::EmbeddedImagePtr> &embedded)
{
    d->embeddedImages = embedded;
}

// --------------------- Operators -------------------//

QDataStream &KIdentityManagement::operator<<(QDataStream &stream, const KIdentityManagement::Signature &sig)
{
    return stream << static_cast<quint8>(sig.type()) << sig.path() << sig.text() << sig.imageLocation() << sig.embeddedImages() << sig.isEnabledSignature();
}

QDataStream &KIdentityManagement::operator>>(QDataStream &stream, KIdentityManagement::Signature &sig)
{
    quint8 s;
    QString path;
    QString text;
    QString saveLocation;
    QVector<Signature::EmbeddedImagePtr> lst;
    bool enabled;
    stream >> s >> path >> text >> saveLocation >> lst >> enabled;
    sig.setText(text);
    sig.setPath(path);
    sig.setImageLocation(saveLocation);
    sig.setEmbeddedImages(lst);
    sig.setEnabledSignature(enabled);
    sig.setType(static_cast<Signature::Type>(s));
    return stream;
}

bool Signature::operator==(const Signature &other) const
{
    if (d->type != other.type()) {
        return false;
    }

    if (d->enabled != other.isEnabledSignature()) {
        return false;
    }

    if (d->type == Inlined && d->inlinedHtml) {
        if (d->saveLocation != other.imageLocation()) {
            return false;
        }
        if (d->embeddedImages != other.embeddedImages()) {
            return false;
        }
    }

    switch (d->type) {
    case Inlined:
        return d->text == other.text();
    case FromFile:
    case FromCommand:
        return d->path == other.path();
    default:
    case Disabled:
        return true;
    }
}

QString Signature::toPlainText() const
{
    QString sigText = rawText();
    if (!sigText.isEmpty() && isInlinedHtml() && type() == Inlined) {
        // Use a QTextDocument as a helper, it does all the work for us and
        // strips all HTML tags.
        QTextDocument helper;
        QTextCursor helperCursor(&helper);
        helperCursor.insertHtml(sigText);
        sigText = helper.toPlainText();
    }
    return sigText;
}

void Signature::addImage(const QImage &imageData, const QString &imageName)
{
    Q_ASSERT(!(d->saveLocation.isEmpty()));
    Signature::EmbeddedImagePtr image(new Signature::EmbeddedImage());
    image->image = imageData;
    image->name = imageName;
    d->embeddedImages.append(image);
}

void Signature::setImageLocation(const QString &path)
{
    d->saveLocation = path;
}

QString Signature::imageLocation() const
{
    return d->saveLocation;
}

// --------------- Getters -----------------------//

QString Signature::text() const
{
    return d->text;
}

QString Signature::path() const
{
    return d->path;
}

Signature::Type Signature::type() const
{
    return d->type;
}

// --------------- Setters -----------------------//

void Signature::setText(const QString &text)
{
    d->text = text;
    d->type = Inlined;
}

void Signature::setType(Type type)
{
    d->type = type;
}

void Signature::setEnabledSignature(bool enabled)
{
    d->enabled = enabled;
}

bool Signature::isEnabledSignature() const
{
    return d->enabled;
}
