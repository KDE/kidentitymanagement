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
 */
class KIDENTITYMANAGEMENTCORE_EXPORT IdentityTreeModel : public QAbstractListModel
{
    Q_OBJECT

public:
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
     */
    explicit IdentityTreeModel(IdentityManager *manager, QObject *parent = nullptr);
    /*!
     */
    ~IdentityTreeModel() override;

    /*!
     */
    [[nodiscard]] QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    /*!
     */
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    /*!
     */
    [[nodiscard]] int columnCount(const QModelIndex &parent) const override;
    /*!
     */
    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    /*!
     */
    void setShowDefault(bool show);

    /*!
     */
    [[nodiscard]] uint identityUoid(int index) const;
    /*!
     */
    [[nodiscard]] int uoidIndex(int uoid) const;

    /*!
     */
    [[nodiscard]] KIdentityManagementCore::IdentityManager *identityManager() const;

    /*!
     */
    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex &index) const override;
    /*!
     */
    [[nodiscard]] bool setData(const QModelIndex &modelIndex, const QVariant &value, int role = Qt::DisplayRole) override;

    /*!
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
