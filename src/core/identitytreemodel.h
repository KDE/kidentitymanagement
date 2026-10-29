// SPDX-FileCopyrightText: 2021 Carl Schwan <carlschwan@kde.org>
// SPDX-FileCopyrightText: 2023 Claudio Cambra <claudio.cambra@kde.org>
// SPDX-FileCopyrightText: 2024-2026 Laurent Montel <montel@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#pragma once

#include "kidentitymanagementcore_export.h"

#include <QAbstractListModel>

#include "identitymanager.h"

namespace KIdentityManagementCore
{
class IdentityManager;
/*!
 * \class KIdentityManagementCore::IdentityTreeModel
 * \inmodule KIdentityManagementCore
 * \inheaderfile KIdentityManagementCore/IdentityTreeModel
 *
 * \brief The IdentityTreeModel class.
 *
 * \author Laurent Montel <montel@kde.org>
 */
class KIDENTITYMANAGEMENTCORE_EXPORT IdentityTreeModel : public QAbstractListModel
{
    Q_OBJECT

public:
    /*!
     * \enum KIdentityManagementCore::IdentityTreeModel::IdentityRoles
     *
     * Columns (and data roles) provided by the model.
     *
     * \value IdentityNameRole The name of the identity
     * \value DisplayIdentityNameRole The name used to display the identity
     * \value FullEmailRole The full email address (name and address)
     * \value EmailRole The primary email address of the identity
     * \value UoidRole The unique object identifier of the identity
     * \value DefaultRole Whether the identity is the default one
     * \value ActivitiesRole The activities the identity is associated with
     * \value EnabledActivitiesRole Whether activities are enabled for the identity
     * \value LastColumn The last column of the model
     */
    enum IdentityRoles {
        IdentityNameRole,
        DisplayIdentityNameRole,
        FullEmailRole,
        EmailRole,
        UoidRole,
        DefaultRole,
        ActivitiesRole,
        EnabledActivitiesRole,
        LastColumn = EnabledActivitiesRole,
    };

    /*!
     * Constructor
     * \a manager the identity manager to use
     * \a parent the parent object
     */
    explicit IdentityTreeModel(IdentityManager *manager, QObject *parent = nullptr);
    /*!
     * Destructor
     */
    ~IdentityTreeModel() override;

    /*!
     * Gets the data for the given model index
     * \a index the model index to get data for
     * \a role the data role to retrieve
     * \return the data at the given index for the specified role
     */
    [[nodiscard]] QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    /*!
     * Returns the number of rows under the given parent
     * \a parent the parent model index
     * \return the number of rows
     */
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    /*!
     * Returns the number of columns under the given parent
     * \a parent the parent model index
     * \return the number of columns
     */
    [[nodiscard]] int columnCount(const QModelIndex &parent) const override;
    /*!
     * Returns the header data for the given section
     * \a section the column or row number
     * \a orientation whether the section is horizontal (columns) or vertical (rows)
     * \a role the data role to retrieve
     * \return the header data
     */
    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    /*!
     * Sets whether to show the default identity
     * \a show true to show the default identity with "(default)" suffix
     */
    void setShowDefault(bool show);

    /*!
     * Returns the UOID (Unique Object Identifier) of the identity at the given index
     * \a index the row index
     * \return the UOID of the identity at that index
     */
    [[nodiscard]] uint identityUoid(int index) const;
    /*!
     * Returns the row index for the identity with the given UOID
     * \a uoid the unique object identifier to search for
     * \return the row index of the identity, or -1 if not found
     */
    [[nodiscard]] int uoidIndex(int uoid) const;

    /*!
     * Returns the underlying identity manager
     * \return the identity manager
     */
    [[nodiscard]] KIdentityManagementCore::IdentityManager *identityManager() const;

    /*!
     * Returns the item flags for the given index
     * \a index the model index
     * \return the item flags
     */
    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex &index) const override;
    /*!
     * Sets the data for the given index
     * \a modelIndex the model index to set data for
     * \a value the value to set
     * \a role the data role to set
     * \return true if the data was successfully set, false otherwise
     */
    [[nodiscard]] bool setData(const QModelIndex &modelIndex, const QVariant &value, int role = Qt::DisplayRole) override;

    /*!
     * Removes the identities with the given names
     * \a identitiesName a list of identity names to remove
     */
    void removeIdentities(const QStringList &identitiesName);

private:
    KIDENTITYMANAGEMENTCORE_NO_EXPORT void reloadUoidList();
    KIDENTITYMANAGEMENTCORE_NO_EXPORT QString generateIdentityName(const KIdentityManagementCore::Identity &identity) const;
    QList<int> mIdentitiesUoid;
    bool mShowDefault = false;
    KIdentityManagementCore::IdentityManager *const mIdentityManager;
};
}
