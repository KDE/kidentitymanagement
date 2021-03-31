/*  -*- c++ -*-
    SPDX-FileCopyrightText: 2008 Thomas McGuire <Thomas.McGuire@gmx.net>
    SPDX-FileCopyrightText: 2008 Edwin Schepers <yez@familieschepers.nl>
    SPDX-FileCopyrightText: 2008 Tom Albers <tomalbers@kde.nl>
    SPDX-FileCopyrightText: 2004 Marc Mutz <mutz@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#pragma once

#include "kidentitymanagement_export.h"
#include "signature.h" // for Signature::Type
#include <QWidget>

using KIdentityManagement::Signature;

namespace KIdentityManagement
{
/**
 * This widget gives an interface so users can edit their signature.
 * You can set a signature via setSignature(), let the user edit the
 * signature and when done, read the signature back.
 */
class KIDENTITYMANAGEMENT_EXPORT SignatureConfigurator : public QWidget
{
    Q_OBJECT
public:
    /**
     * Constructor
     */
    explicit SignatureConfigurator(QWidget *parent = nullptr);

    /**
     * destructor
     */
    ~SignatureConfigurator() override;

    /**
     * Enum for the different viewmodes.
     */
    enum ViewMode { ShowCode, ShowHtml };

    /**
     * Indicated if the user wants a signature
     */
    Q_REQUIRED_RESULT bool isSignatureEnabled() const;

    /**
     * Use this to activate the signature.
     */
    void setSignatureEnabled(bool enable);

    /**
     * This returns the type of the signature,
     * so that can be Disabled, Inline, fromFile, etc.
     */
    Q_REQUIRED_RESULT Signature::Type signatureType() const;

    /**
     * Set the signature type to @p type.
     */
    void setSignatureType(Signature::Type type);

    /**
     * Make @p text the text for the signature.
     */
    void setInlineText(const QString &text);

    /**
     * Returns the file url which the user wants
     * to use as a signature.
     */
    Q_REQUIRED_RESULT QString filePath() const;

    /**
     * Set @p url for the file url part of the
     * widget.
     */
    void setFileURL(const QString &url);

    /**
     * Returns the url of the command which the
     * users wants to use as signature.
     */
    Q_REQUIRED_RESULT QString commandPath() const;

    /**
     * Sets @p url as the command to execute.
     */
    void setCommandURL(const QString &url);

    /**
       Convenience method.
       @return a Signature object representing the state of the widgets.
     **/
    Q_REQUIRED_RESULT Signature signature() const;

    /**
       Convenience method. Sets the widgets according to @p sig
       @param sig the signature to configure
    **/
    void setSignature(const Signature &sig);

    /**
     * Sets the directory where the images used in the HTML signature will be stored.
     * Needs to be called before calling setSignature(), as each signature should use
     * a different location.
     * The directory needs to exist, it will not be created.
     * @param path the image location to set
     * @since 4.4
     * @sa Signature::setImageLocation
     */
    void setImageLocation(const QString &path);

    /**
     * Sets the image location to the image location of a given identity, which is
     * emailidentities/<identity-id>/.
     *
     * @param identity The identity whose unique ID will be used to determine the image
     *                 location.
     * @since 4.4
     */
    void setImageLocation(const Identity &identity);

private:
    void slotUrlChanged();
    void slotEdit();
    void slotSetHtml();

    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond
};
}

