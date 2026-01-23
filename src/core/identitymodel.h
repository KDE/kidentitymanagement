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
     */
    explicit IdentityModel(QObject *parent = nullptr);
    /*!
     */
    ~IdentityModel() override;

    /*!
     */
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    /*!
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    /*!
     */
    QHash<int, QByteArray> roleNames() const override;

    /*!
     * Returns the email address of the identity with the given uoid.
     * \a uiod for the identity in question
     */
    Q_INVOKABLE QString email(uint uoid);

private:
    KIDENTITYMANAGEMENTCORE_NO_EXPORT void reloadUoidList();
    QList<int> m_identitiesUoid;
    IdentityManager *const m_identityManager;
};
}
