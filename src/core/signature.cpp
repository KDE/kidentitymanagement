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
#include <KProcess>

#include <QDir>
#include <QTextBlock>
#include <QTextDocument>

#include <cassert>

using namespace KIdentityManagement;

class KIdentityManagement::SignaturePrivate
{
public:
    explicit SignaturePrivate(Signature *qq)
        : q(qq)
    {
    }

    void assignFrom(const KIdentityManagement::Signature &that);
    void cleanupImages();
    void saveImages() const;
    Q_REQUIRED_RESULT QString textFromFile(bool *ok) const;
    Q_REQUIRED_RESULT QString textFromCommand(bool *ok, QString *errorMessage) const;

    /// List of images that belong to this signature. Either added by addImage() or
    /// by readConfig().
    QList<Signature::EmbeddedImagePtr> embeddedImages;

    /// The directory where the images will be saved to.
    QString saveLocation;
    QString path;
    QString text;
    Signature::Type type = Signature::Disabled;
    bool enabled = false;
    bool inlinedHtml = false;
    Signature *const q;
};

// Returns the names of all images in the HTML code
static QStringList findImageNames(const QString &htmlCode)
{
    QStringList imageNames;
    QTextDocument doc;
    doc.setHtml(htmlCode);
    for (auto block = doc.begin(); block.isValid(); block = block.next()) {
        for (auto it = block.begin(); !it.atEnd(); ++it) {
            const auto fragment = it.fragment();
            if (fragment.isValid()) {
                const auto imageFormat = fragment.charFormat().toImageFormat();
                if (imageFormat.isValid() && !imageFormat.name().startsWith(QLatin1String("http")) && !imageNames.contains(imageFormat.name())) {
                    imageNames.push_back(imageFormat.name());
                }
            }
        }
    }
    return imageNames;
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
        return {};
    }

    if (ok) {
        *ok = true;
    }
    const QByteArray ba = f.readAll();
    return QString::fromLocal8Bit(ba.data(), ba.size());
}

QString SignaturePrivate::textFromCommand(bool *ok, QString *errorMessage) const
{
    assert(type == Signature::FromCommand);

    // handle pathological cases:
    if (path.isEmpty()) {
        if (ok) {
            *ok = true;
        }
        return {};
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
        if (errorMessage) {
            *errorMessage = i18n(
                "<qt>Failed to execute signature script<p><b>%1</b>:</p>"
                "<p>%2</p></qt>",
                path,
                QString::fromUtf8(proc.readAllStandardError()));
        }
        return {};
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

QDataStream &operator<<(QDataStream &stream, const KIdentityManagement::Signature::EmbeddedImagePtr &img)
{
    return stream << img->image << img->name;
}

QDataStream &operator>>(QDataStream &stream, const KIdentityManagement::Signature::EmbeddedImagePtr &img)
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

Signature::~Signature() = default;

QString Signature::rawText(bool *ok, QString *errorMessage) const
{
    switch (d->type) {
    case Disabled:
        if (ok) {
            *ok = true;
        }
        return {};
    case Inlined:
        if (ok) {
            *ok = true;
        }
        return d->text;
    case FromFile:
        return d->textFromFile(ok);
    case FromCommand:
        return d->textFromCommand(ok, errorMessage);
    }
    qCritical() << "Signature::type() returned unknown value!";
    return {}; // make compiler happy
}

QString Signature::withSeparator(bool *ok, QString *errorMessage) const
{
    QString signature = rawText(ok, errorMessage);
    if (ok && (*ok) == false) {
        return {};
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

QList<Signature::EmbeddedImagePtr> Signature::embeddedImages() const
{
    return d->embeddedImages;
}

void Signature::setEmbeddedImages(const QList<Signature::EmbeddedImagePtr> &embedded)
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
    QList<Signature::EmbeddedImagePtr> lst;
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
