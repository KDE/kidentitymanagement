/*
    SPDX-FileCopyrightText: 2002 Marc Mutz <mutz@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kidentitymanagementcore_export.h"

#include <QList>
#include <QObject>
#include <QStringList>

#include <memory>

namespace KIdentityManagementCore
{
class IdentityManagerPrivate;
class Identity;
/*!
 * \class KIdentityManagementCore::IdentityManager
 * \inmodule KIdentityManagementCore
 * \inheaderfile KIdentityManagementCore/IdentityManager
 *
 * \brief Manages the list of identities.
 * \author Marc Mutz <mutz@kde.org>
 **/
class KIDENTITYMANAGEMENTCORE_EXPORT IdentityManager : public QObject
{
    Q_OBJECT
public:
    /*!
     * Create an identity manager, which loads the emailidentities file
     * to create identities.
     * \a readonly if true, no changes can be made to the identity manager
     * This means in particular that if there is no identity configured,
     * the default identity created here will not be saved.
     * It is assumed that a minimum of one identity is always present.
     */
    explicit IdentityManager(bool readonly = false, QObject *parent = nullptr, const char *name = nullptr);
    ~IdentityManager() override;

    /*!
     * Creates or reuses the identity manager instance for this process.
     * It loads the emailidentities file to create identities.
     * This sets readonly to false, so you should create a separate instance
     * if you need it to be readonly.
     * \since 5.2.91
     */
    static IdentityManager *self();

public:
    using Iterator = QList<Identity>::Iterator;
    using ConstIterator = QList<Identity>::ConstIterator;

    /*!
     * Typedef for STL style iterator
     */
    using iterator = Iterator;

    /*!
     * Typedef for STL style iterator
     */
    using const_iterator = ConstIterator;

    /*! Returns a unique name for a new identity based on \a name
     *  \a name the name of the base identity
     */
    [[nodiscard]] QString makeUnique(const QString &name) const;

    /*! Returns whether the \a name is unique
     *  \a name the name to be examined
     */
    [[nodiscard]] bool isUnique(const QString &name) const;

    /*! Commit changes to disk and emit changed() if necessary. */
    void commit();

    /*! Re-read the config from disk and forget changes. */
    void rollback();

    /*! Store a new identity or modify an existing identity based on an
     *  independent identity object
     *  \a ident the identity to be saved
     */
    void saveIdentity(const Identity &ident);

    /*! Check whether there are any unsaved changes. */
    [[nodiscard]] bool hasPendingChanges() const;

    /*! Returns the list of identities */
    [[nodiscard]] QStringList identities() const;

    /*! Convenience method.

        Returns the list of (shadow) identities, ie. the ones currently
        under configuration.
    */
    [[nodiscard]] QStringList shadowIdentities() const;

    /*! Sort the identities by name (the default is always first). This
        operates on the @em shadow list, so you need to \ commit for
        the changes to take effect.
    **/
    void sort();

    /*! Returns an identity whose address matches any in \a addresses
                or \ Identity::null if no such identity exists.
        \a addresses the string of addresses to scan for matches
    **/
    const Identity &identityForAddress(const QString &addresses) const;

    /*! Returns true if \a addressList contains any of our addresses,
                false otherwise.
        \a addressList the addressList to examine
        \sa #identityForAddress
    **/
    [[nodiscard]] bool thatIsMe(const QString &addressList) const;

    /*! Returns the identity with Unique Object Identifier (UOID) @p
                uoid or \ Identity::null if not found.
        \a uoid the Unique Object Identifier to find identity with
     **/
    const Identity &identityForUoid(uint uoid) const;

    /*! Convenience method.

        Returns the identity with Unique Object Identifier (UOID) @p
                uoid or the default identity if not found.
        \a uoid the Unique Object Identifier to find identity with
    **/
    const Identity &identityForUoidOrDefault(uint uoid) const;

    /*! Returns the default identity */
    const Identity &defaultIdentity() const;

    /*! Sets the identity with Unique Object Identifier (UOID) \a uoid
        to be new the default identity. As usual, use \ commit to
        make this permanent.

        \a uoid the default identity to set
        Returns false if an identity with UOID \a uoid was not found
    **/
    bool setAsDefault(uint uoid);

    /*! Returns the identity named \a identityName. This method returns a
        reference to the identity that can be modified. To let others
        see this change, use \ commit.
        \a identityName the identity name to return modifiable reference
    **/
    Identity &modifyIdentityForName(const QString &identityName);

    /*! Returns the identity with Unique Object Identifier (UOID) \a uoid.
        This method returns a reference to the identity that can
        be modified. To let others see this change, use \ commit.
    **/
    Identity &modifyIdentityForUoid(uint uoid);

    /*! Removes the identity with name \a identityName
        Will return false if the identity is not found,
        or when one tries to remove the last identity.
        \a identityName the identity to remove
     **/
    [[nodiscard]] bool removeIdentity(const QString &identityName);

    /*!
     * Removes the identity with name \a identityName
     * Will return \\ false if the identity is not found, \\ true otherwise.
     *
     * \
ote In opposite to removeIdentity, this method allows to remove the
     *       last remaining identity.
     *
     * \since 4.6
     */
    [[nodiscard]] bool removeIdentityForced(const QString &identityName);

    ConstIterator begin() const;
    ConstIterator end() const;
    /// Iterator used by the configuration dialog, which works on a separate list
    /// of identities, for modification. Changes are made effective by commit().
    Iterator modifyBegin();
    Iterator modifyEnd();

    Identity &newFromScratch(const QString &name);
    Identity &newFromControlCenter(const QString &name);
    Identity &newFromExisting(const Identity &other, const QString &name = QString());

    /*! Returns the list of all email addresses (only name@host) from all
        identities */
    [[nodiscard]] QStringList allEmails() const;

Q_SIGNALS:
    /*! Emitted whenever a commit changes any configure option */
    void changed();
    void identitiesWereChanged();
    /*! Emitted whenever the identity with Unique Object Identifier
        (UOID) \a uoid changed. Useful for more fine-grained change
        notifications than what is possible with the standard \
        changed() signal. */
    void changed(uint uoid);
    /*! Emitted whenever the identity \a ident changed. Useful for more
        fine-grained change notifications than what is possible with the
        standard \ changed() signal. */
    void changed(const KIdentityManagementCore::Identity &ident);
    void identityChanged(const KIdentityManagementCore::Identity &ident);
    /*! Emitted on \ commit() for each deleted identity. At the time
        this signal is emitted, the identity does still exist and can be
        retrieved by \ identityForUoid() if needed */
    void deleted(uint uoid);
    /*! Emitted on \ commit() for each new identity */
    void added(const KIdentityManagementCore::Identity &ident);

    void needToReloadIdentitySettings();

    void identitiesChanged(const QString &id);

protected:
    /*!
     * This is called when no identity has been defined, so we need to
     * create a default one. The parameters are filled with some default
     * values from KUser, but reimplementations of this method can give
     * them another value.
     */
    virtual void createDefaultIdentity(QString & /*fullName*/, QString & /*emailAddress*/);

protected Q_SLOTS:
    void slotRollback();

private:
    friend class IdentityManagerPrivate;
    std::unique_ptr<IdentityManagerPrivate> const d;
    Q_PRIVATE_SLOT(d, void slotIdentitiesChanged(const QString &id))
};
} // namespace
