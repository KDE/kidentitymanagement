/*
    Copyright (c) 2002-2004 Marc Mutz <mutz@kde.org>
    Copyright (c) 2007 Tom Albers <tomalbers@kde.nl>
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

#include "kpimidentities_export.h"

#include <kdemacros.h>

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QList>
#include <QtCore/QHash>
#include <QtCore/QVariant>

namespace KPIMIdentities
{
  class Signature;
}
class KConfigGroup;
class KRichTextEdit;

namespace KPIMIdentities
{

  KPIMIDENTITIES_EXPORT QDataStream &operator<<
  ( QDataStream &stream, const KPIMIdentities::Signature &sig );
  KPIMIDENTITIES_EXPORT QDataStream &operator>>
  ( QDataStream &stream, KPIMIdentities::Signature &sig );

  /**
   * @short abstraction of a signature (aka "footer").
   * @author Marc Mutz <mutz@kde.org>
   */
  class KPIMIDENTITIES_EXPORT Signature
  {
      friend class Identity;

      friend KPIMIDENTITIES_EXPORT QDataStream &operator<< ( QDataStream &stream, const Signature &sig );
      friend KPIMIDENTITIES_EXPORT QDataStream &operator>> ( QDataStream &stream, Signature &sig );

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
      bool operator== ( const Signature &other ) const;

      /** Constructor for disabled signature */
      Signature();
      /** Constructor for inline text */
      Signature( const QString &text );
      /** Constructor for text from a file or from output of a command */
      Signature( const QString &url, bool isExecutable );

      /** @return the raw signature text as entered resp. read from file. */
      QString rawText( bool *ok=0 ) const;

      /** @return the signature text with a "-- \n" separator added, if
          necessary. A newline will not be appended or prepended. */
      QString withSeparator( bool *ok=0 ) const;

      /** Set the signature text and mark this signature as being of
          "inline text" type. */
      void setText( const QString &text );
      QString text() const;

      /** Set the signature URL and mark this signature as being of
          "from file" resp. "from output of command" type. */
      void setUrl( const QString &url, bool isExecutable=false );
      QString url() const;

      /// @return the type of signature (ie. way to obtain the signature text)
      Type type() const;
      void setType( Type type );

      /**
       * Sets the inlined signature to text or html
       * @param isHtml sets the inlined signature to html
       * @since 4.1
       */
      void setInlinedHtml( bool isHtml );

      /**
       * @return boolean whether the inlined signature is html
       * @since 4.1
       */
      bool isInlinedHtml() const;


      /**
       * Inserts this signature into the given text edit.
       * The cursor position is preserved.
       * A leading or trailing newline is also added automatically, depending on
       * the placement.
       * For undo/redo, this is treated as one operation.
       *
       * Rich text mode of the text edit will be enabled if the signature is in
       * inlined HTML format.
       *
       * @param textEdit the signature will be inserted into this text edit.
       * @param placement defines where in the text edit the signature should be
       *                  inserted.
       * @param addSeparator if true, the separator '-- \n' will be added in front
       *                     of the signature
       *
       * @since 4.3
       */
      void insertIntoTextEdit( KRichTextEdit *textEdit,
                               Placement placement = End, bool addSeparator = true );

      /**
       * Inserts this given signature into the given text edit.
       * The cursor position is preserved.
       * A leading or trailing newline is also added automatically, depending on
       * the placement.
       * For undo/redo, this is treated as one operation.
       * A separator is not added.
       *
       * Use the insertIntoTextEdit() function if possible, as it has support
       * for separators and does HTML detection automatically.
       *
       * Rich text mode of the text edit will be enabled if @p isHtml is true.
       *
       * @param signature the signature, either as plain text or as HTML
       * @param textEdit the text edit to insert the signature into
       * @param placement defines where in the textedit the signature should be
       *                  inserted.
       * @param isHtml defines whether the signature should be inserted as text or html
       *
       * @since 4.3
       */
      static void insertPlainSignatureIntoTextEdit( const QString &signature,
                                                    KRichTextEdit *textEdit,
                                                    Placement placement = End,
                                                    bool isHtml = false );

    protected:
      void writeConfig( KConfigGroup &config ) const;
      void readConfig( const KConfigGroup &config );

    private:
      QString textFromFile( bool *ok ) const;
      QString textFromCommand( bool *ok ) const;

      QString mUrl;
      QString mText;
      Type    mType;
      bool mInlinedHtml;
  };

}

#endif /*kpim_signature_h*/
