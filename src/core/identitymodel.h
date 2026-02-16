// SPDX-FileCopyrightText: 2021 Carl Schwan <carlschwan@kde.org>
// SPDX-FileCopyrightText: 2023 Claudio Cambra <claudio.cambra@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#pragma once

#include "kidentitymanagementcore_export.h"

#include <QAbstractListModel>

#include "identitymanager.h"

namespace KIdentityManagementCore
{

class IdentityManager;
/*!
 * \class KIdentityManagementCore::IdentityModel
 * \inmodule KIdentityManagementCore
 * \inheaderfile KIdentityManagementCore/IdentityModel
 *
 * \brief The IdentityModel class
 * \author Laurent Montel <montel@kde.org>
 */
class KIDENTITYMANAGEMENTCORE_EXPORT IdentityModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        EmailRole = Qt::UserRole,
        UoidRole,
        IdentityNameRole,
        DisplayNameRole,
        DefaultRole,
    };
    /*!
     * Constructor
     * \param parent the parent object
     */
    explicit IdentityModel(QObject *parent = nullptr);
    /*!
     * Destructor
     */
    ~IdentityModel() override;

    /*!
     * Gets the data for the given model index
     * \param index the model index to get data for
     * \param role the data role to retrieve
     * \return the data at the given index for the specified role
     */
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    /*!
     * Returns the number of rows under the given parent
     * \param parent the parent model index
     * \return the number of rows
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    /*!
     * Returns the role names for the model
     * \return a hash of role names
     */
    QHash<int, QByteArray> roleNames() const override;

    /*!
     * Returns the email address of the identity with the given uoid
     * \param uoid the unique object identifier for the identity in question
     * \return the email address for the identity
     */
    Q_INVOKABLE QString email(uint uoid);

private:
    KIDENTITYMANAGEMENTCORE_NO_EXPORT void reloadUoidList();
    QList<int> m_identitiesUoid;
    IdentityManager *const m_identityManager;
};
}
