// SPDX-FileCopyrightText: 2021 Carl Schwan <carlschwan@kde.org>
// SPDX-FileCopyrightText: 2023 Claudio Cambra <claudio.cambra@kde.org>
// SPDX-FileCopyrightText: 2024 Laurent Montel <montel@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#pragma once

#include "kidentitymanagementcore_export.h"

#include <QAbstractListModel>

#include "identitymanager.h"

namespace KIdentityManagementCore
{

class IdentityManager;

class KIDENTITYMANAGEMENTCORE_EXPORT IdentityModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum IdentityRoles {
        EmailRole,
        UoidRole,
        IdentityNameRole,
        DisplayNameRole,
        DefaultRole,
        LastColumn = DefaultRole,
    };

    explicit IdentityModel(QObject *parent = nullptr);
    ~IdentityModel();

    [[nodiscard]] QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
    [[nodiscard]] int columnCount(const QModelIndex &parent) const override;

    /**
     * @return the email address of the identity with the given uoid.
     * @param uiod for the identity in question
     */
    Q_INVOKABLE QString email(uint uoid);

private:
    KIDENTITYMANAGEMENTCORE_NO_EXPORT void reloadUoidList();
    QList<int> m_identitiesUoid;
    IdentityManager *const m_identityManager;
};
}
