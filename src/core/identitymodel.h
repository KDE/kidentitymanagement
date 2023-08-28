// SPDX-FileCopyrightText: 2021 Carl Schwan <carlschwan@kde.org>
// SPDX-FileCopyrightText: 2023 Claudio Cambra <claudio.cambra@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#pragma once

#include "kidentitymanagement_export.h"

#include <QAbstractListModel>

namespace KIdentityManagement
{

class IdentityManager;

class KIDENTITYMANAGEMENT_EXPORT IdentityModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles { EmailRole = Qt::UserRole, UoidRole, IdentityNameRole };

    IdentityModel(QObject *parent = nullptr);
    ~IdentityModel();

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    /**
     * @return the email address of the identity with the given uoid.
     * @param uiod for the identity in question
     */
    Q_INVOKABLE QString email(uint uoid);

private Q_SLOTS:
    void reloadUoidList();

private:
    QList<int> m_identitiesUoid;
    IdentityManager *const m_identityManager;
};
}
