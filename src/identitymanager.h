/*
    SPDX-FileCopyrightText: 2002 Marc Mutz <mutz@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <KConfigGroup>
#include <QObject>
#include <QVector>
#include <kidentitymanagement_export.h>

#include <QStringList>

namespace KIdentityManagement
{
class Identity;
/**
 * @short Manages the list of identities.
 * @author Marc Mutz <mutz@kde.org>
 **/
class KIDENTITYMANAGEMENT_EXPORT IdentityManager : public QObject
{
    Q_OBJECT
public:
    /**
     * Create an identity manager, which loads the emailidentities file
     * to create identities.
     * @param readonly if true, no changes can be made to the identity manager
     * This means in particular that if there is no identity configured,
     * the default identity created here will not be saved.
     * It is assumed that a minimum of one identity is always present.
     */
    explicit IdentityManager(bool readonly = false, QObject *parent = nullptr, const char *name = nullptr);
    ~IdentityManager() override;

    /**
     * Creates or reuses the identity manager instance for this process.
     * It loads the emailidentities file to create identities.
     * This sets readonly to false, so you should create a separate instance
     * if you need it to be readonly.
     * @since 5.2.91
     */
    static IdentityManager *self();

public:
    using Iterator = QVector<Identity>::Iterator;
    using ConstIterator = QVector<Identity>::ConstIterator;

    /**
     * Typedef for STL style iterator
     */
    using iterator = Iterator;

    /**
     * Typedef for STL style iterator
     */
    using const_iterator = ConstIterator;

    /** @return a unique name for a new identity based on @p name
     *  @param name the name of the base identity
     */
    Q_REQUIRED_RESULT QString makeUnique(const QString &name) const;

    /** @return whether the @p name is unique
     *  @param name the name to be examined
     */
    Q_REQUIRED_RESULT bool isUnique(const QString &name) const;

    /** Commit changes to disk and emit changed() if necessary. */
    void commit();

    /** Re-read the config from disk and forget changes. */
    void rollback();

    /** Check whether there are any unsaved changes. */
    Q_REQUIRED_RESULT bool hasPendingChanges() const;

    /** @return the list of identities */
    Q_REQUIRED_RESULT QStringList identities() const;

    /** Convenience method.

        @return the list of (shadow) identities, ie. the ones currently
        under configuration.
    */
    Q_REQUIRED_RESULT QStringList shadowIdentities() const;

    /** Sort the identities by name (the default is always first). This
        operates on the @em shadow list, so you need to @ref commit for
        the changes to take effect.
    **/
    void sort();

    /** @return an identity whose address matches any in @p addresses
                or @ref Identity::null if no such identity exists.
        @param addresses the string of addresses to scan for matches
    **/
    const Identity &identityForAddress(const QString &addresses) const;

    /** @return true if @p addressList contains any of our addresses,
                false otherwise.
        @param addressList the addressList to examine
        @see #identityForAddress
    **/
    Q_REQUIRED_RESULT bool thatIsMe(const QString &addressList) const;

    /** @return the identity with Unique Object Identifier (UOID) @p
                uoid or @ref Identity::null if not found.
        @param uoid the Unique Object Identifier to find identity with
     **/
    const Identity &identityForUoid(uint uoid) const;

    /** Convenience method.

        @return the identity with Unique Object Identifier (UOID) @p
                uoid or the default identity if not found.
        @param uoid the Unique Object Identifier to find identity with
    **/
    const Identity &identityForUoidOrDefault(uint uoid) const;

    /** @return the default identity */
    const Identity &defaultIdentity() const;

    /** Sets the identity with Unique Object Identifier (UOID) @p uoid
        to be new the default identity. As usual, use @ref commit to
        make this permanent.

        @param uoid the default identity to set
        @return false if an identity with UOID @p uoid was not found
    **/
    Q_REQUIRED_RESULT bool setAsDefault(uint uoid);

    /** @return the identity named @p identityName. This method returns a
        reference to the identity that can be modified. To let others
        see this change, use @ref commit.
        @param identityName the identity name to return modifiable reference
    **/
    Identity &modifyIdentityForName(const QString &identityName);

    /** @return the identity with Unique Object Identifier (UOID) @p uoid.
        This method returns a reference to the identity that can
        be modified. To let others see this change, use @ref commit.
    **/
    Identity &modifyIdentityForUoid(uint uoid);

    /** Removes the identity with name @p identityName
        Will return false if the identity is not found,
        or when one tries to remove the last identity.
        @param identityName the identity to remove
     **/
    Q_REQUIRED_RESULT bool removeIdentity(const QString &identityName);

    /**
     * Removes the identity with name @p identityName
     * Will return @c false if the identity is not found, @c true otherwise.
     *
     * @note In opposite to removeIdentity, this method allows to remove the
     *       last remaining identity.
     *
     * @since 4.6
     */
    Q_REQUIRED_RESULT bool removeIdentityForced(const QString &identityName);

    ConstIterator begin() const;
    ConstIterator end() const;
    /// Iterator used by the configuration dialog, which works on a separate list
    /// of identities, for modification. Changes are made effective by commit().
    Iterator modifyBegin();
    Iterator modifyEnd();

    Identity &newFromScratch(const QString &name);
    Identity &newFromControlCenter(const QString &name);
    Identity &newFromExisting(const Identity &other, const QString &name = QString());

    /** Returns the list of all email addresses (only name@host) from all
        identities */
    Q_REQUIRED_RESULT QStringList allEmails() const;

Q_SIGNALS:
    /** Emitted whenever a commit changes any configure option */
    void changed();
    /** Emitted whenever the identity with Unique Object Identifier
        (UOID) @p uoid changed. Useful for more fine-grained change
        notifications than what is possible with the standard @ref
        changed() signal. */
    void changed(uint uoid);
    /** Emitted whenever the identity @p ident changed. Useful for more
        fine-grained change notifications than what is possible with the
        standard @ref changed() signal. */
    void changed(const KIdentityManagement::Identity &ident);
    /** Emitted on @ref commit() for each deleted identity. At the time
        this signal is emitted, the identity does still exist and can be
        retrieved by @ref identityForUoid() if needed */
    void deleted(uint uoid);
    /** Emitted on @ref commit() for each new identity */
    void added(const KIdentityManagement::Identity &ident);

    void needToReloadIdentitySettings();

protected:
    /**
     * This is called when no identity has been defined, so we need to
     * create a default one. The parameters are filled with some default
     * values from KUser, but reimplementations of this method can give
     * them another value.
     */
    virtual void createDefaultIdentity(QString & /*fullName*/, QString & /*emailAddress*/);

protected Q_SLOTS:
    void slotRollback();

Q_SIGNALS:
    void identitiesChanged(const QString &id);

private:
    //@cond PRIVATE
    class Private;
    Private *d;
    //@endcond
    Q_PRIVATE_SLOT(d, void slotIdentitiesChanged(const QString &id))
};
} // namespace

