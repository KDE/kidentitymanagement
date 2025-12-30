// SPDX-FileCopyrightText: 2024-2025 Laurent Montel <montel@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#pragma once
#include "kidentitymanagementcore_export.h"

#include <QSortFilterProxyModel>

namespace KIdentityManagementCore
{
class IdentityActivitiesAbstract;
/*!
 * \class KIdentityManagementCore::IdentityTreeSortProxyModel
 * \inmodule KIdentityManagementCore
 * \inheaderfile KIdentityManagementCore/IdentityTreeSortProxyModel
 */
class KIDENTITYMANAGEMENTCORE_EXPORT IdentityTreeSortProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    /*!
     */
    explicit IdentityTreeSortProxyModel(QObject *parent);
    /*!
     */
    ~IdentityTreeSortProxyModel() override;

    /*!
     */
    [[nodiscard]] IdentityActivitiesAbstract *identityActivitiesAbstract() const;
    /*!
     */
    void setIdentityActivitiesAbstract(IdentityActivitiesAbstract *newIdentityActivitiesAbstract);

    /*!
     */
    [[nodiscard]] bool enablePlasmaActivities() const;
    /*!
     */
    void setEnablePlasmaActivities(bool newEnablePlasmaActivities);

protected:
    /*!
     */
    [[nodiscard]] bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    KIDENTITYMANAGEMENTCORE_NO_EXPORT void slotInvalidateFilter();
    IdentityActivitiesAbstract *mIdentityActivitiesAbstract = nullptr;
    bool mEnablePlasmaActivities = false;
};
}
