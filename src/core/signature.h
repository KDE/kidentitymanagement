/*
    SPDX-FileCopyrightText: 2002-2004 Marc Mutz <mutz@kde.org>
    SPDX-FileCopyrightText: 2007 Tom Albers <tomalbers@kde.nl>
    SPDX-FileCopyrightText: 2009 Thomas McGuire <mcguire@kde.org>
    Author: Stefan Taferner <taferner@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kidentitymanagementcore_export.h"

#include <QImage>
#include <QSharedPointer>
#include <QString>
#include <memory>

class KConfigGroup;
namespace KIdentityManagementCore
{
class Signature;
class Identity;
class SignaturePrivate;
KIDENTITYMANAGEMENTCORE_EXPORT QDataStream &operator<<(QDataStream &stream, const KIdentityManagementCore::Signature &sig);
KIDENTITYMANAGEMENTCORE_EXPORT QDataStream &operator>>(QDataStream &stream, KIdentityManagementCore::Signature &sig);

/*!
 * \class KIdentityManagementCore::Signature
 * \inmodule KIdentityManagementCore
 * \inheaderfile KIdentityManagementCore/Signature
 *
 * \brief Abstraction of a signature (aka "footer").
 *
 * The signature can either be plain text, HTML text, text returned from a command or text stored
 * in a file.
 *
 * In case of HTML text, the signature can contain images.
 * Since you set the HTML source with setText(), there also needs to be a way to add the images
 * to the signature, as the HTML source contains only the img tags that reference those images.
 * To add the image to the signature, call addImage(). The name given there must match the name
 * of the img tag in the HTML source.
 *
 * The images need to be stored somewhere. The Signature class handles that by storing all images
 * in a directory. You must set that directory with setImageLocation(), before calling addImage().
 * The images added with addImage() are then saved to that directory when calling writeConfig().
 * When loading a signature, readConfig() automatically loads the images as well.
 * To actually add the images to a text edit, call insertIntoTextEdit().
 *
 * Example of creating a HTML signature and then inserting it into a text edit:
 * \code
 * Signature htmlSig;
 * htmlSig.setText( "<img src=\"hello.png\"> World" );
 * htmlSig.setInlinedHtml( true );
 * htmlSig.setImageLocation( KStandardDirs::locateLocal( "data", "emailidentities/example/" );
 * QImage image = ...;
 * htmlSig.addImage( image, "hello.png" );
 * ...
 * KTextEdit edit;
 * htmlSig.insertIntoTextEdit( KIdentityManagementCore::Signature::End,
 *                             KIdentityManagementCore::Signature::AddSeparator, &edit );
 * \endcode
 */
class KIDENTITYMANAGEMENTCORE_EXPORT Signature
{
    friend class Identity;

    friend KIDENTITYMANAGEMENTCORE_EXPORT QDataStream &operator<<(QDataStream &stream, const Signature &sig);
    friend KIDENTITYMANAGEMENTCORE_EXPORT QDataStream &operator>>(QDataStream &stream, Signature &sig);

public:
    /*!
     * Type of signature (ie. way to obtain the signature text)
     *
     * \value Disabled No signature
     * \value Inlined The signature text is stored in the configuration
     * \value FromFile The signature text is read from a file
     * \value FromCommand The signature text is the output of a command
     */
    enum Type {
        Disabled = 0,
        Inlined = 1,
        FromFile = 2,
        FromCommand = 3,
    };

    /*!
     * Describes the placement of the signature text when it is to be inserted into a
     * text edit
     *
     * \value Start The signature is placed at the start of the textedit
     * \value End The signature is placed at the end of the textedit
     * \value AtCursor The signature is placed at the current cursor position
     */
    enum Placement {
        Start,
        End,
        AtCursor
    };

    struct EmbeddedImage {
        QImage image;
        QString name;
    };
    using EmbeddedImagePtr = QSharedPointer<EmbeddedImage>;

    /*! Returns \c true if this signature is equal to \a other. */
    bool operator==(const Signature &other) const;

    /*! Constructor for disabled signature */
    Signature();
    /*! Constructor for inline text */
    Signature(const QString &text);
    /*! Constructor for text from the file \a path or, if \a isExecutable is \c true, from the output of the command \a path */
    Signature(const QString &path, bool isExecutable);
    /*! Copy constructor */
    Signature(const Signature &that);
    /*! Assigns \a that to this signature. */
    Signature &operator=(const Signature &that);
    /*! Destructor */
    ~Signature();

    /*! Returns the raw signature text as entered resp. read from file.
        \a ok set to \c true if reading succeeded
        \a errorMessage If available, contains a human readable explanation for \a ok being \c false.
     */
    [[nodiscard]] QString rawText(bool *ok = nullptr, QString *errorMessage = nullptr) const;

    /*! Returns the signature text with a "-- \n" separator added, if
        necessary. A newline will not be appended or prepended.
        \a ok set to \c true if reading succeeded
        \a errorMessage If available, contains a human readable explanation for \a ok being \c false.
     */
    [[nodiscard]] QString withSeparator(bool *ok = nullptr, QString *errorMessage = nullptr) const;

    /*! Set the signature text to \a text and mark this signature as being of
        "inline text" type. */
    void setText(const QString &text);
    /*! Returns the inlined signature text. */
    [[nodiscard]] QString text() const;

    /*!
     * Returns the text of the signature. If the signature is HTML, the HTML
     * tags will be stripped.
     * \since 4.4
     */
    [[nodiscard]] QString toPlainText() const;

    /*! Set the signature URL to \a path and mark this signature as being of
        "from file" resp. "from output of command" type, depending on \a isExecutable. */
    void setPath(const QString &path, bool isExecutable = false);
    /*! Returns the path of the signature file or command. */
    [[nodiscard]] QString path() const;

    /*! Returns the type of signature (ie. way to obtain the signature text) */
    [[nodiscard]] Type type() const;
    /*! Sets the type of signature to \a type. */
    void setType(Type type);

    /*!
     * Sets the inlined signature to text or html
     * \a isHtml sets the inlined signature to html
     * \since 4.1
     */
    void setInlinedHtml(bool isHtml);

    /*!
     * Returns boolean whether the inlined signature is html
     * \since 4.1
     */
    [[nodiscard]] bool isInlinedHtml() const;

    /*!
     * Sets the location where the copies of the signature images will be stored.
     * The images will be stored there when calling writeConfig(). The image location
     * is stored in the config, so the next readConfig() call knows where to look for
     * images.
     * It is recommended to use KStandardDirs::locateLocal( "data", "emailidentities/%1" )
     * for the location, where %1 is the unique identifier of the identity.
     *
     * \warning readConfig will delete all other PNG files in this directory, as they could
     *          be stale inline image files
     *
     * Like with addImage(), the SignatureConfigurator will handle this for you.
     * \a path the path to set as image location
     * \since 4.4
     */
    void setImageLocation(const QString &path);
    /*!
     * Returns the location where the copies of the signature images are stored.
     * \sa setImageLocation()
     * \since 4.4
     */
    [[nodiscard]] QString imageLocation() const;

    /*!
     * Adds the given \a image to the signature.
     * This is needed if you use setText() to set some HTML source that references images. Those
     * referenced images needed to be added by calling this function. The \a imageName has to match
     * the src attribute of the img tag.
     *
     * If you use SignatureConfigurator, you don't need to call this function, as the configurator
     * will handle this for you.
     * setImageLocation() needs to be called once before.
     * \since 4.4
     */
    void addImage(const QImage &image, const QString &imageName);

    /*!
     * \brief setEnabledSignature
     * \a enabled enables signature if set as \c true
     * \since 4.9
     */
    void setEnabledSignature(bool enabled);
    /*!
     * Returns \c true if the signature is enabled.
     * \since 4.9
     */
    [[nodiscard]] bool isEnabledSignature() const;

    /*!
     * Describes which additional parts should be added to the signature.
     *
     * \value AddNothing Don't add any text to the signature
     * \value AddSeparator The separator '-- \\n' will be added in front of the signature
     * \value AddNewLines Add a newline character in front or after the signature, depending on the placement
     */
    enum AddedTextFlag {
        AddNothing = 0,
        AddSeparator = 1 << 0,
        AddNewLines = 1 << 1,
    };

    using AddedText = QFlags<AddedTextFlag>;

    /*! Returns the list of images embedded in the signature. */
    [[nodiscard]] QList<Signature::EmbeddedImagePtr> embeddedImages() const;
    /*! Sets the list of images embedded in the signature to \a embedded. */
    void setEmbeddedImages(const QList<EmbeddedImagePtr> &embedded);

protected:
    // TODO: KDE5: BIC: Move all to private class
    /*! Writes the signature to the configuration group \a config. */
    void writeConfig(KConfigGroup &config) const;
    /*! Reads the signature from the configuration group \a config. */
    void readConfig(const KConfigGroup &config);

private:
    std::unique_ptr<SignaturePrivate> const d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Signature::AddedText)
}
