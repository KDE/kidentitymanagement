// SPDX-FileCopyrightText: 2021 Carl Schwan <carlschwan@kde.org>
// SPDX-FileCopyrightText: 2023 Claudio Cambra <claudio.cambra@kde.org>
// SPDX-FileCopyrightText: 2024 Laurent Montel <montel@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#pragma once

#include "kidentitymanagementwidgets_export.h"

#include <QAbstractListModel>

#include "identitymanager.h"

namespace KIdentityManagementCore
{
class IdentityManager;
}
namespace KIdentityManagementWidgets
{
class KIDENTITYMANAGEMENTWIDGETS_EXPORT IdentityModel : public QAbstractListModel
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
    ~IdentityModel() override;

    [[nodiscard]] QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] int columnCount(const QModelIndex &parent) const override;

private:
    KIDENTITYMANAGEMENTWIDGETS_NO_EXPORT void reloadUoidList();
    QList<int> mIdentitiesUoid;
    KIdentityManagementCore::IdentityManager *const mIdentityManager;
};
}
