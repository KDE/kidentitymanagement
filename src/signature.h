/*
    SPDX-FileCopyrightText: 2002-2004 Marc Mutz <mutz@kde.org>
    SPDX-FileCopyrightText: 2007 Tom Albers <tomalbers@kde.nl>
    SPDX-FileCopyrightText: 2009 Thomas McGuire <mcguire@kde.org>
    Author: Stefan Taferner <taferner@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kidentitymanagement_export.h"

#include <QHash>
#include <QImage>
#include <QList>
#include <QString>

namespace KIdentityManagement
{
class Signature;
class Identity;
}
class KConfigGroup;

namespace KPIMTextEdit
{
class RichTextComposer;
}

namespace KIdentityManagement
{
class SignaturePrivate;
KIDENTITYMANAGEMENT_EXPORT QDataStream &operator<<(QDataStream &stream, const KIdentityManagement::Signature &sig);
KIDENTITYMANAGEMENT_EXPORT QDataStream &operator>>(QDataStream &stream, KIdentityManagement::Signature &sig);

/**
 * @short Abstraction of a signature (aka "footer").
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
 * @code
 * Signature htmlSig;
 * htmlSig.setText( "<img src=\"hello.png\"> World" );
 * htmlSig.setInlinedHtml( true );
 * htmlSig.setImageLocation( KStandardDirs::locateLocal( "data", "emailidentities/example/" );
 * QImage image = ...;
 * htmlSig.addImage( image, "hello.png" );
 * ...
 * KTextEdit edit;
 * htmlSig.insertIntoTextEdit( KIdentityManagement::Signature::End,
 *                             KIdentityManagement::Signature::AddSeparator, &edit );
 * @endcode
 */
class KIDENTITYMANAGEMENT_EXPORT Signature
{
    friend class Identity;

    friend KIDENTITYMANAGEMENT_EXPORT QDataStream &operator<<(QDataStream &stream, const Signature &sig);
    friend KIDENTITYMANAGEMENT_EXPORT QDataStream &operator>>(QDataStream &stream, Signature &sig);

public:
    /** Type of signature (ie. way to obtain the signature text) */
    enum Type { Disabled = 0, Inlined = 1, FromFile = 2, FromCommand = 3 };

    /**
     * Describes the placement of the signature text when it is to be inserted into a
     * text edit
     */
    enum Placement {
        Start, ///< The signature is placed at the start of the textedit
        End, ///< The signature is placed at the end of the textedit
        AtCursor ///< The signature is placed at the current cursor position
    };

    struct EmbeddedImage {
        QImage image;
        QString name;
    };
    using EmbeddedImagePtr = QSharedPointer<EmbeddedImage>;

    /** Used for comparison */
    bool operator==(const Signature &other) const;

    /** Constructor for disabled signature */
    Signature();
    /** Constructor for inline text */
    Signature(const QString &text);
    /** Constructor for text from a file or from output of a command */
    Signature(const QString &path, bool isExecutable);
    /** Copy constructor */
    Signature(const Signature &that);
    /** Assignment operator */
    Signature &operator=(const Signature &that);
    /** Destructor */
    ~Signature();

    /** @return the raw signature text as entered resp. read from file.
        @param ok set to @c true if reading succeeded
     */
    Q_REQUIRED_RESULT QString rawText(bool *ok = nullptr) const;

    /** @return the signature text with a "-- \n" separator added, if
        necessary. A newline will not be appended or prepended.
        @param ok set to @c true if reading succeeded
     */
    Q_REQUIRED_RESULT QString withSeparator(bool *ok = nullptr) const;

    /** Set the signature text and mark this signature as being of
        "inline text" type. */
    void setText(const QString &text);
    Q_REQUIRED_RESULT QString text() const;

    /**
     * Returns the text of the signature. If the signature is HTML, the HTML
     * tags will be stripped.
     * @since 4.4
     */
    Q_REQUIRED_RESULT QString toPlainText() const;

    /** Set the signature URL and mark this signature as being of
        "from file" resp. "from output of command" type. */
    void setPath(const QString &path, bool isExecutable = false);
    Q_REQUIRED_RESULT QString path() const;

    /// @return the type of signature (ie. way to obtain the signature text)
    Q_REQUIRED_RESULT Type type() const;
    void setType(Type type);

    /**
     * Sets the inlined signature to text or html
     * @param isHtml sets the inlined signature to html
     * @since 4.1
     */
    void setInlinedHtml(bool isHtml);

    /**
     * @return boolean whether the inlined signature is html
     * @since 4.1
     */
    Q_REQUIRED_RESULT bool isInlinedHtml() const;

    /**
     * Sets the location where the copies of the signature images will be stored.
     * The images will be stored there when calling writeConfig(). The image location
     * is stored in the config, so the next readConfig() call knows where to look for
     * images.
     * It is recommended to use KStandardDirs::locateLocal( "data", "emailidentities/%1" )
     * for the location, where %1 is the unique identifier of the identity.
     *
     * @warning readConfig will delete all other PNG files in this directory, as they could
     *          be stale inline image files
     *
     * Like with addImage(), the SignatureConfigurator will handle this for you.
     * @param path the path to set as image location
     * @since 4.4
     */
    void setImageLocation(const QString &path);
    Q_REQUIRED_RESULT QString imageLocation() const;

    /**
     * Adds the given image to the signature.
     * This is needed if you use setText() to set some HTML source that references images. Those
     * referenced images needed to be added by calling this function. The @imageName has to match
     * the src attribute of the img tag.
     *
     * If you use SignatureConfigurator, you don't need to call this function, as the configurator
     * will handle this for you.
     * setImageLocation() needs to be called once before.
     * @since 4.4
     */
    void addImage(const QImage &image, const QString &imageName);

    /**
     * @brief setEnabledSignature
     * @param enabled enables signature if set as @c true
     * @since 4.9
     */
    void setEnabledSignature(bool enabled);
    Q_REQUIRED_RESULT bool isEnabledSignature() const;

    enum AddedTextFlag {
        AddNothing = 0, ///< Don't add any text to the signature
        AddSeparator = 1 << 0, ///< The separator '-- \n' will be added in front
        ///  of the signature
        AddNewLines = 1 << 1 ///< Add a newline character in front or after the signature,
                             ///  depending on the placement
    };

    /// Describes which additional parts should be added to the signature
    using AddedText = QFlags<AddedTextFlag>;

    /** Inserts this signature into the given text edit.
     * If the signature is inserted at the beginning, a couple of new
     * lines will be inserted before it, and the cursor is moved to
     * the beginning. Otherwise, the cursor position is preserved.
     * For undo/redo, this is treated as one operation.
     *
     * Rich text mode of the text edit will be enabled if the signature is in
     * inlined HTML format.
     *
     * If this signature uses images, they will be added automatically.
     *
     * @param placement defines where in the text edit the signature should be
     *                  inserted.
     * @param addedText defines which other texts should be added to the signature
     * @param textEdit the signature will be inserted into this text edit.
     *
     * @since 4.9
     */
    // TODO: KDE5: BIC: Reorder parameters, the order here is a workaround for ambiguous parameters
    //                  with the deprecated method
    void insertIntoTextEdit(Placement placement, AddedText addedText, KPIMTextEdit::RichTextComposer *textEdit, bool forceDisplay = false) const;

    Q_REQUIRED_RESULT QVector<Signature::EmbeddedImagePtr> embeddedImages() const;
    void setEmbeddedImages(const QVector<EmbeddedImagePtr> &embedded);

protected:
    // TODO: KDE5: BIC: Move all to private class
    void writeConfig(KConfigGroup &config) const;
    void readConfig(const KConfigGroup &config);

private:
    //@cond PRIVATE
    SignaturePrivate *const d;
    //@endcond
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Signature::AddedText)
}

