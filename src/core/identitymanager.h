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
 */
class KIDENTITYMANAGEMENTCORE_EXPORT IdentityManager : public QObject
{
    Q_OBJECT
public:
    /*!
     * Constructor
     * \param readonly if true, no changes can be made to the identity manager
     *                 This means in particular that if there is no identity configured,
     *                 the default identity created here will not be saved.
     *                 It is assumed that a minimum of one identity is always present.
     * \param parent the parent object
     * \param name the object name
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
     *  \param name the name of the base identity
     *  \return a unique identity name
     */
    [[nodiscard]] QString makeUnique(const QString &name) const;

    /*! Returns whether the \a name is unique
     *  \param name the name to be examined
     *  \return true if the name is unique, false otherwise
     */
    [[nodiscard]] bool isUnique(const QString &name) const;

    /*! Commit changes to disk and emit changed() if necessary */
    void commit();

    /*! Re-read the config from disk and forget changes */
    void rollback();

    /*! Store a new identity or modify an existing identity based on an independent identity object
     *  \param ident the identity to be saved
     */
    void saveIdentity(const Identity &ident);

    /*! Check whether there are any unsaved changes
     *  \return true if there are pending changes, false otherwise
     */
    [[nodiscard]] bool hasPendingChanges() const;

    /*! Returns the list of identities
     *  \return a list of identity names
     */
    [[nodiscard]] QStringList identities() const;

    /*! Convenience method. Returns the list of (shadow) identities, i.e. the ones currently
        under configuration.
        \return a list of shadow identity names
    */
    [[nodiscard]] QStringList shadowIdentities() const;

    /*! Sort the identities by name (the default is always first). This
        operates on the shadow list, so you need to call commit() for
        the changes to take effect.
    */
    void sort();

    /*! Returns an identity whose address matches any in \a addresses or Identity::null if no such identity exists.
        \param addresses the string of addresses to scan for matches
        \return the identity matching the addresses or Identity::null
    */
    const Identity &identityForAddress(const QString &addresses) const;

    /*! Returns true if \a addressList contains any of our addresses, false otherwise.
        \param addressList the addressList to examine
        \return true if one of our addresses is found in the list
        \sa identityForAddress()
    */
    [[nodiscard]] bool thatIsMe(const QString &addressList) const;

    /*! Returns the identity with Unique Object Identifier (UOID) \a uoid or Identity::null if not found.
        \param uoid the Unique Object Identifier to find identity with
        \return the identity with the given UOID or Identity::null
     */
    const Identity &identityForUoid(uint uoid) const;

    /*! Convenience method. Returns the identity with Unique Object Identifier (UOID) \a uoid
        or the default identity if not found.
        \param uoid the Unique Object Identifier to find identity with
        \return the identity with the given UOID or the default identity
    */
    const Identity &identityForUoidOrDefault(uint uoid) const;

    /*! Returns the default identity
     *  \return the default identity
     */
    const Identity &defaultIdentity() const;

    /*! Sets the identity with Unique Object Identifier (UOID) \a uoid to be the new default identity.
        As usual, use commit() to make this permanent.

        \param uoid the default identity to set
        \return false if an identity with UOID \a uoid was not found, true otherwise
    */
    bool setAsDefault(uint uoid);

    /*! Returns the identity named \a identityName. This method returns a reference to the identity
        that can be modified. To let others see this change, use commit().
        \param identityName the identity name to return modifiable reference for
        \return a modifiable reference to the identity
    */
    Identity &modifyIdentityForName(const QString &identityName);

    /*! Returns the identity with Unique Object Identifier (UOID) \a uoid. This method returns a
        reference to the identity that can be modified. To let others see this change, use commit().
        \param uoid the Unique Object Identifier to find identity with
        \return a modifiable reference to the identity
    */
    Identity &modifyIdentityForUoid(uint uoid);

    /*! Removes the identity with name \a identityName
        Will return false if the identity is not found, or when one tries to remove the last identity.
        \param identityName the identity to remove
        \return true if the identity was removed, false otherwise
     */
    bool removeIdentity(const QString &identityName);

    /*! Removes the identity with name \a identityName
     * Will return false if the identity is not found, true otherwise.
     *
     * Note: In opposite to removeIdentity(), this method allows to remove the
     *       last remaining identity.
     *
     * \param identityName the identity to remove
     * \return true if the identity was removed, false if not found
     * \since 4.6
     */
    [[nodiscard]] bool removeIdentityForced(const QString &identityName);

    /*! \return a const iterator pointing to the first identity in the list */
    ConstIterator begin() const;
    /*! \return a const iterator pointing past the last identity in the list */
    ConstIterator end() const;
    /*! Iterator used by the configuration dialog, which works on a separate list
     * of identities, for modification. Changes are made effective by commit().
     * \return a mutable iterator pointing to the first identity in the shadow list
     */
    Iterator modifyBegin();
    /*! \return a mutable iterator pointing past the last identity in the shadow list */
    Iterator modifyEnd();

    /*! Creates a new identity with the given name from scratch
     *  \param name the name for the new identity
     *  \return a reference to the newly created identity
     */
    Identity &newFromScratch(const QString &name);
    /*! Creates a new identity with the given name, copying values from the Control Center defaults
     *  \param name the name for the new identity
     *  \return a reference to the newly created identity
     */
    Identity &newFromControlCenter(const QString &name);
    /*! Creates a new identity with the given name, copying all values from an existing identity
     *  \param other the identity to copy values from
     *  \param name the name for the new identity (if empty, uses other's name)
     *  \return a reference to the newly created identity
     */
    Identity &newFromExisting(const Identity &other, const QString &name = QString());

    /*! Returns the list of all email addresses (only name@host) from all identities
     *  \return a list of all email addresses across all identities
     */
    [[nodiscard]] QStringList allEmails() const;

Q_SIGNALS:
    /*! Emitted whenever a commit changes any configuration option */
    void changed();
    /*! Emitted whenever any identity changes */
    void identitiesWereChanged();
    /*! Emitted whenever the identity with Unique Object Identifier (UOID) \a uoid changed.
        Useful for more fine-grained change notifications than what is possible with the changed() signal.
        \param uoid the unique object identifier of the changed identity
    */
    void changed(uint uoid);
    /*! Emitted whenever the identity \a ident changed.
        Useful for more fine-grained change notifications than what is possible with the changed() signal.
        \param ident the identity that changed
    */
    void changed(const KIdentityManagementCore::Identity &ident);
    /*! Emitted whenever an identity changes
     *  \param ident the identity that changed
     */
    void identityChanged(const KIdentityManagementCore::Identity &ident);
    /*! Emitted on commit() for each deleted identity.
        At the time this signal is emitted, the identity does still exist and can be
        retrieved by identityForUoid() if needed.
        \param uoid the unique object identifier of the deleted identity
    */
    void deleted(uint uoid);
    /*! Emitted on commit() for each new identity
     *  \param ident the newly added identity
     */
    void added(const KIdentityManagementCore::Identity &ident);

    /*! Emitted when identity settings need to be reloaded */
    void needToReloadIdentitySettings();

    /*! Emitted when identities change
     *  \param id the identity identifier
     */
    void identitiesChanged(const QString &id);

protected:
    /*!
     * This is called when no identity has been defined, so we need to create a default one.
     * The parameters are filled with some default values from KUser, but reimplementations
     * of this method can give them another value.
     * \param fullName out parameter for the full name
     * \param emailAddress out parameter for the email address
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
