/*
    Copyright (c) 2002-2004 Marc Mutz <mutz@kde.org>
    Copyright (c) 2007 Tom Albers <tomalbers@kde.nl>
    Copyright (c) 2009 Thomas McGuire <mcguire@kde.org>
    Author: Stefan Taferner <taferner@kde.org>

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

#ifndef KPIMIDENTITIES_SIGNATURE_H
#define KPIMIDENTITIES_SIGNATURE_H

#include "kidentitymanagement_export.h"

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QList>
#include <QtCore/QHash>
#include <QtCore/QVariant>

namespace KIdentityManagement
{
class Signature;
class Identity;
}
class KConfigGroup;
class KRichTextEdit;

namespace KPIMTextEdit
{
class TextEdit;
}

namespace KIdentityManagement
{

KIDENTITYMANAGEMENT_EXPORT QDataStream &operator<<
(QDataStream &stream, const KIdentityManagement::Signature &sig);
KIDENTITYMANAGEMENT_EXPORT QDataStream &operator>>
(QDataStream &stream, KIdentityManagement::Signature &sig);

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

    friend KIDENTITYMANAGEMENT_EXPORT QDataStream &operator<< (QDataStream &stream, const Signature &sig);
    friend KIDENTITYMANAGEMENT_EXPORT QDataStream &operator>> (QDataStream &stream, Signature &sig);

public:
    /** Type of signature (ie. way to obtain the signature text) */
    enum Type {
        Disabled = 0,
        Inlined = 1,
        FromFile = 2,
        FromCommand = 3
    };

    /**
     * Describes the placement of the signature text when it is to be inserted into a
     * text edit
     */
    enum Placement {
        Start,                   ///< The signature is placed at the start of the textedit
        End,                     ///< The signature is placed at the end of the textedit
        AtCursor                 ///< The signature is placed at the current cursor position
    };

    /** Used for comparison */
    bool operator== (const Signature &other) const;

    /** Constructor for disabled signature */
    Signature();
    /** Constructor for inline text */
    Signature(const QString &text);
    /** Constructor for text from a file or from output of a command */
    Signature(const QString &url, bool isExecutable);
    /** Copy constructor */
    Signature(const Signature &that);
    /** Assignment operator */
    Signature &operator= (const Signature &that);
    /** Destructor */
    ~Signature();

    /** @return the raw signature text as entered resp. read from file.
        @param ok set to @c true if reading succeeded
     */
    QString rawText(bool *ok = 0) const;

    /** @return the signature text with a "-- \n" separator added, if
        necessary. A newline will not be appended or prepended.
        @param ok set to @c true if reading succeeded
     */
    QString withSeparator(bool *ok = 0) const;

    /** Set the signature text and mark this signature as being of
        "inline text" type. */
    void setText(const QString &text);
    QString text() const;

    /**
     * Returns the text of the signature. If the signature is HTML, the HTML
     * tags will be stripped.
     * @since 4.4
     */
    QString toPlainText() const;

    /** Set the signature URL and mark this signature as being of
        "from file" resp. "from output of command" type. */
    void setUrl(const QString &url, bool isExecutable = false);
    QString url() const;

    /// @return the type of signature (ie. way to obtain the signature text)
    Type type() const;
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
    bool isInlinedHtml() const;

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
    bool isEnabledSignature() const;

    enum AddedTextFlag {
        AddNothing = 0,         ///< Don't add any text to the signature
        AddSeparator = 1 << 0,  ///< The separator '-- \n' will be added in front
        ///  of the signature
        AddNewLines = 1 << 1    ///< Add a newline character in front or after the signature,
                      ///  depending on the placement
    };

    /// Describes which additional parts should be added to the signature
    typedef QFlags<AddedTextFlag> AddedText;

    /** Inserts this signature into the given text edit.
      * The cursor position is preserved.
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
    //TODO; KDE5 merge with previous method
    void insertIntoTextEdit(Placement placement, AddedText addedText,
                            KPIMTextEdit::TextEdit *textEdit, bool forceDisplay = false) const;

protected:

    // TODO: KDE5: BIC: Move all to private class
    void writeConfig(KConfigGroup &config) const;
    void readConfig(const KConfigGroup &config);

private:
    void insertSignatureText(Placement placement, AddedText addedText, KPIMTextEdit::TextEdit *textEdit, bool forceDisplay) const;

    // TODO: KDE5: BIC: Move all to private class

    /**
     * Helper used for the copy constructor and the assignment operator
     */
    void assignFrom(const Signature &that);

    /**
     * Clean up unused images from our internal list and delete all images
     * from the file system
     */
    void cleanupImages() const;

    /**
     * Saves all images from our internal list to the file system.
     */
    void saveImages() const;

    QString textFromFile(bool *ok) const;
    QString textFromCommand(bool *ok) const;

    // TODO: KDE5: BIC: Add a d-pointer!!!
    //       There is already a pseude private class in the .cpp, using a hash.
    QString mUrl;
    QString mText;
    Type    mType;
    bool mInlinedHtml;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Signature::AddedText)

}

#endif /*kpim_signature_h*/
