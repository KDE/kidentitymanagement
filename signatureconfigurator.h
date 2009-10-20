/*  -*- c++ -*-
    Copyright 2008 Thomas McGuire <Thomas.McGuire@gmx.net>
    Copyright 2008 Edwin Schepers <yez@familieschepers.nl>
    Copyright 2008 Tom Albers <tomalbers@kde.nl>
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

#ifndef KPIMIDENTITIES_SIGNATURECONFIGURATOR_H
#define KPIMIDENTITIES_SIGNATURECONFIGURATOR_H

#include "kpimidentities_export.h"
#include "signature.h" // for Signature::Type
#include <QtGui/QWidget>

using KPIMIdentities::Signature;

class QCheckBox;
class KComboBox;
class KUrlRequester;
class KLineEdit;
class KToolBar;
class KRichTextWidget;
class QString;
class QPushButton;
class QTextEdit;
class QTextCharFormat;

namespace KPIMIdentities {

  /**
    * This widget gives an interface so users can edit their signature.
    * You can set a signature via setSignature(), let the user edit the
    * signature and when done, read the signature back.
    */
class KPIMIDENTITIES_EXPORT SignatureConfigurator : public QWidget
{
  Q_OBJECT
  public:
     /**
       * Constructor
       */
    explicit SignatureConfigurator( QWidget * parent = 0 );

    /**
      * destructor
      */
    virtual ~SignatureConfigurator();

    /**
      * Enum for the different viemodes.
      */
    enum ViewMode { ShowCode, ShowHtml };

    /**
      * Indicated if the user wants a signature
      */
    bool isSignatureEnabled() const;

    /**
      * Use this to activate the signature.
      */
    void setSignatureEnabled( bool enable );

    /**
      * This returns the type of the signature,
      * so that can be Disabled, Inline, fromFile, etc.
      */
    Signature::Type signatureType() const;

    /**
      * Set the signature type to @p type.
      */
    void setSignatureType( Signature::Type type );

    /**
      * Returns the inline text, only useful
      * when this is the appropriate Signature::Type
      */
    QString inlineText() const;

    /**
      * Make @p text the text for the signature.
      */
    void setInlineText( const QString & text );

    /**
      * Returns the file url which the user wants
      * to use as a signature.
      */
    QString fileURL() const;

    /**
      * Set @p url for the file url part of the
      * widget.
      */
    void setFileURL( const QString & url );

    /**
      * Returns the url of the command which the
      * users wants to use as signature.
      */
    QString commandURL() const;

    /**
      * Sets @p url as the command to execute.
      */
    void setCommandURL( const QString & url );

    /**
       Conveniece method.
       @return a Signature object representing the state of the widgets.
     **/
    Signature signature() const;

    /**
       Convenience method. Sets the widgets according to @p sig
    **/
    void setSignature( const Signature & sig );

    /**
     * Sets the directory where the images used in the HTML signature will be stored.
     * Needs to be called before calling setSignature(), as each signature should use
     * a different location.
     * The directory needs to exist, it will not be created.
     * @since 4.4
     * @sa Signature::setImageLocation
     */
    void setImageLocation( const QString &path );

  private:
    void toggleHtmlBtnState( ViewMode state );

    void initHtmlState();

    // Returns the current text of the textedit as HTML code, but strips
    // unnecessary tags Qt inserts
    QString asCleanedHTML() const;

  protected Q_SLOTS:
    void slotEnableEditButton( const QString & );
    void slotEdit();
    void slotSetHtml();

  protected:

    // TODO: KDE5: BIC: Move to private class!
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

  private:
    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond
};

}

#endif
