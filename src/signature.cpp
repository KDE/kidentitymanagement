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

using namespace KIdentityManagement;



class KIdentityManagement::Signature::Private
{
public:
    Private(Signature *qq)
        : enabled(false),
          q(qq)
    {
    }

    /// List of images that belong to this signature. Either added by addImage() or
    /// by readConfig().
    QList<Signature::EmbeddedImagePtr> embeddedImages;

    /// The directory where the images will be saved to.
    QString saveLocation;
    bool enabled;
    QString url;
    QString text;
    Signature::Type type;
    bool inlinedHtml;
    Signature *q;
};

QDataStream &operator<< (QDataStream &stream, const KIdentityManagement::Signature::EmbeddedImagePtr &img)
{
    return stream << img->image << img->name;
}

QDataStream &operator>> (QDataStream &stream, KIdentityManagement::Signature::EmbeddedImagePtr &img)
{
    return stream >> img->image >> img->name;
}

Signature::Signature()
    : d(new Private(this))
{
    d->type = Disabled;
    d->inlinedHtml = false;
}

Signature::Signature(const QString &text)
    : d(new Private(this))
{
    d->type = Inlined;
    d->inlinedHtml = false;
    d->text = text;
}

Signature::Signature(const QString &url, bool isExecutable)
    : d(new Private(this))
{
    d->type = isExecutable ? FromCommand : FromFile;
    d->url = url;
}

void Signature::assignFrom(const KIdentityManagement::Signature &that)
{
    d->url = that.url();
    d->inlinedHtml = that.isInlinedHtml();
    d->text = that.text();
    d->type = that.type();
    d->enabled = that.isEnabledSignature();
    d->saveLocation = that.imageLocation();
    d->embeddedImages = that.embeddedImages();
}

Signature::Signature(const Signature &that)
    : d(new Private(this))
{
    assignFrom(that);
}

Signature &Signature::operator= (const KIdentityManagement::Signature &that)
{
    if (this == &that) {
        return *this;
    }

    assignFrom(that);
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
        return textFromFile(ok);
    case FromCommand:
        return textFromCommand(ok);
    };
    qCritical() << "Signature::type() returned unknown value!";
    return QString(); // make compiler happy
}

QString Signature::textFromCommand(bool *ok) const
{
    assert(d->type == FromCommand);

    // handle pathological cases:
    if (d->url.isEmpty()) {
        if (ok) {
            *ok = true;
        }
        return QString();
    }

    // create a shell process:
    KProcess proc;
    proc.setOutputChannelMode(KProcess::SeparateChannels);
    proc.setShellCommand(d->url);
    int rc = proc.execute();

    // handle errors, if any:
    if (rc != 0) {
        if (ok) {
            *ok = false;
        }
        const QString wmsg = i18n("<qt>Failed to execute signature script<p><b>%1</b>:</p>"
                                  "<p>%2</p></qt>", d->url, QLatin1String(proc.readAllStandardError()));
        KMessageBox::error(0, wmsg);
        return QString();
    }

    // no errors:
    if (ok) {
        *ok = true;
    }

    // get output:
    QByteArray output = proc.readAllStandardOutput();

    // TODO: hmm, should we allow other encodings, too?
    return QString::fromLocal8Bit(output.data(), output.size());
}

QString Signature::textFromFile(bool *ok) const
{
    assert(d->type == FromFile);

    // TODO: Use KIO::NetAccess to download non-local files!
    if (!QUrl(d->url).isLocalFile() &&
            !(QFileInfo(d->url).isRelative() &&
              QFileInfo(d->url).exists())) {
        qDebug() << "Signature::textFromFile:"
                 << "non-local URLs are unsupported";
        if (ok) {
            *ok = false;
        }
        return QString();
    }

    if (ok) {
        *ok = true;
    }

    // TODO: hmm, should we allow other encodings, too?
    const QByteArray ba = KPIMUtils::kFileToByteArray(d->url, false);
    return QString::fromLocal8Bit(ba.data(), ba.size());
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
    QString newline = htmlSig ? QLatin1String("<br>") : QLatin1String("\n");
    if (htmlSig && signature.startsWith(QLatin1String("<p"))) {
        newline.clear();
    }

    if (signature.startsWith(QString::fromLatin1("-- ") + newline) ||
            (signature.indexOf(newline + QString::fromLatin1("-- ") + newline) != -1)) {
        // already have signature separator at start of sig or inside sig:
        return signature;
    } else {
        // need to prepend one:
        return QString::fromLatin1("-- ") + newline + signature;
    }
}

void Signature::setUrl(const QString &url, bool isExecutable)
{
    d->url = url;
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

// Returns the names of all images in the HTML code
static QStringList findImageNames(const QString &htmlCode)
{
    QStringList ret;

    // To complicated for us, so cheat and let a text edit do the hard work
    KPIMTextEdit::TextEdit edit;
    edit.setHtml(htmlCode);
    foreach (const KPIMTextEdit::ImageWithNamePtr &image, edit.imagesWithName()) {
        ret << image->name;
    }
    return ret;
}

void Signature::cleanupImages() const
{
    // Remove any images from the internal structure that are no longer there
    if (isInlinedHtml()) {
        foreach (const Signature::EmbeddedImagePtr &imageInList, d->embeddedImages) {
            bool found = false;
            foreach (const QString &imageInHtml, findImageNames(d->text)) {
                if (imageInHtml == imageInList->name) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                d->embeddedImages.removeAll(imageInList);
            }
        }
    }

    // Delete all the old image files
    if (!d->saveLocation.isEmpty()) {
        QDir dir(d->saveLocation);
        foreach (const QString &fileName, dir.entryList(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks)) {
            if (fileName.toLower().endsWith(QLatin1String(".png"))) {
                qDebug() << "Deleting old image" << dir.path() + fileName;
                dir.remove(fileName);
            }
        }
    }
}

void Signature::saveImages() const
{
    if (isInlinedHtml() && !d->saveLocation.isEmpty()) {
        foreach (const Signature::EmbeddedImagePtr &image, d->embeddedImages) {
            QString location = d->saveLocation + QLatin1Char('/') + image->name;
            if (!image->image.save(location, "PNG")) {
                qWarning() << "Failed to save image" << location;
            }
        }
    }
}

void Signature::readConfig(const KConfigGroup &config)
{
    QString sigType = config.readEntry(sigTypeKey);
    if (sigType == QLatin1String(sigTypeInlineValue)) {
        d->type = Inlined;
        d->inlinedHtml = config.readEntry(sigTypeInlinedHtmlKey, false);
    } else if (sigType == QLatin1String(sigTypeFileValue)) {
        d->type = FromFile;
        d->url = config.readPathEntry(sigFileKey, QString());
    } else if (sigType == QLatin1String(sigTypeCommandValue)) {
        d->type = FromCommand;
        d->url = config.readPathEntry(sigCommandKey, QString());
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
        foreach (const QString &fileName, dir.entryList(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks)) {
            if (fileName.toLower().endsWith(QLatin1String(".png"))) {
                QImage image;
                if (image.load(dir.path() + QLatin1Char('/') + fileName)) {
                    addImage(image, fileName);
                } else {
                    qWarning() << "Unable to load image" << dir.path() + QLatin1Char('/') + fileName;
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
        config.writePathEntry(sigFileKey, d->url);
        break;
    case FromCommand:
        config.writeEntry(sigTypeKey, sigTypeCommandValue);
        config.writePathEntry(sigCommandKey, d->url);
        break;
    default:
        break;
    }
    config.writeEntry(sigTextKey, d->text);
    config.writeEntry(sigImageLocation, d->saveLocation);
    config.writeEntry(sigEnabled, d->enabled);

    cleanupImages();
    saveImages();
}

static bool isCursorAtEndOfLine(const QTextCursor &cursor)
{
    QTextCursor testCursor = cursor;
    testCursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    return !testCursor.hasSelection();
}

static void insertSignatureHelper(const QString &signature,
                                  KRichTextEdit *textEdit,
                                  Signature::Placement placement,
                                  bool isHtml,
                                  bool addNewlines)
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
                lineSep = QLatin1String("<br>");
            } else {
                lineSep = QLatin1Char('\n');
            }
        }

        // Insert the signature and newlines depending on where it was inserted.
        bool hackForCursorsAtEnd = false;
        int oldCursorPos = -1;
        if (placement == Signature::End) {

            if (oldCursor.position() == textEdit->toPlainText().length()) {
                hackForCursorsAtEnd = true;
                oldCursorPos = oldCursor.position();
            }
            if (isHtml) {
                textEdit->insertHtml(lineSep + signature);
            } else {
                textEdit->insertPlainText(lineSep + signature);
            }
        } else if (placement == Signature::Start || placement == Signature::AtCursor) {
            if (isHtml) {
                if (isCursorAtEndOfLine(cursor)) {
                    textEdit->insertHtml(signature);
                } else {
                    textEdit->insertHtml(signature + lineSep);
                }
            } else {
                if (isCursorAtEndOfLine(cursor)) {
                    textEdit->insertPlainText(signature);
                } else {
                    textEdit->insertPlainText(signature + lineSep);
                }
            }
        }

        cursor.endEditBlock();

        // There is one special case when re-setting the old cursor: The cursor
        // was at the end. In this case, QTextEdit has no way to know
        // if the signature was added before or after the cursor, and just decides
        // that it was added before (and the cursor moves to the end, but it should
        // not when appending a signature). See bug 167961
        if (hackForCursorsAtEnd) {
            oldCursor.setPosition(oldCursorPos);
        }

        textEdit->setTextCursor(oldCursor);
        textEdit->ensureCursorVisible();

        textEdit->document()->setModified(isModified);

        if (isHtml) {
            textEdit->enableRichTextMode();
        }
    }
}

void Signature::insertIntoTextEdit(Placement placement, AddedText addedText,
                                   KPIMTextEdit::TextEdit *textEdit, bool forceDisplay) const
{
    insertSignatureText(placement, addedText, textEdit, forceDisplay);
}

QList<Signature::EmbeddedImagePtr> Signature::embeddedImages() const
{
    return d->embeddedImages;
}

void Signature::setEmbeddedImages(const QList<Signature::EmbeddedImagePtr> &embedded)
{
    d->embeddedImages = embedded;
}

void Signature::insertSignatureText(Placement placement, AddedText addedText, KPIMTextEdit::TextEdit *textEdit, bool forceDisplay) const
{
    if (!forceDisplay) {
        if (!isEnabledSignature()) {
            return;
        }
    }
    QString signature;
    if (addedText & AddSeparator) {
        signature = withSeparator();
    } else {
        signature = rawText();
    }
    insertSignatureHelper(signature, textEdit, placement,
                          (isInlinedHtml() &&
                           type() == KIdentityManagement::Signature::Inlined),
                          (addedText & AddNewLines));

    // We added the text of the signature above, now it is time to add the images as well.
    if (isInlinedHtml()) {
        foreach (const Signature::EmbeddedImagePtr &image, d->embeddedImages) {
            textEdit->loadImage(image->image, image->name, image->name);
        }
    }
}

// --------------------- Operators -------------------//

QDataStream &KIdentityManagement::operator<<
(QDataStream &stream, const KIdentityManagement::Signature &sig)
{
    return stream << static_cast<quint8>(sig.type()) << sig.url() << sig.text()
           << sig.imageLocation() << sig.embeddedImages() << sig.isEnabledSignature();
}

QDataStream &KIdentityManagement::operator>>
(QDataStream &stream, KIdentityManagement::Signature &sig)
{
    quint8 s;
    QString url;
    QString text;
    QString saveLocation;
    QList<Signature::EmbeddedImagePtr> lst;
    bool enabled;
    stream >> s  >> url >> text >> saveLocation >> lst >> enabled;
    sig.setText(text);
    sig.setUrl(url);
    sig.setImageLocation(saveLocation);
    sig.setEmbeddedImages(lst);
    sig.setEnabledSignature(enabled);
    sig.setType(static_cast<Signature::Type>(s));
    return stream;
}

bool Signature::operator== (const Signature &other) const
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
        return d->url == other.url();
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

QString Signature::url() const
{
    return d->url;
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
