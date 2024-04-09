// SPDX-FileCopyrightText: 2024 Laurent Montel <montel@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#pragma once
#include "kidentitymanagementcore_export.h"

#include <QSortFilterProxyModel>

namespace KIdentityManagementCore
{
class IdentityActivitiesAbstract;
class KIDENTITYMANAGEMENTCORE_EXPORT IdentityTreeSortProxyModel : public QSortFilterProxyModel
{
public:
    explicit IdentityTreeSortProxyModel(QObject *parent);
    ~IdentityTreeSortProxyModel() override;

    [[nodiscard]] IdentityActivitiesAbstract *identityActivitiesAbstract() const;
    void setIdentityActivitiesAbstract(IdentityActivitiesAbstract *newIdentityActivitiesAbstract);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    IdentityActivitiesAbstract *mIdentityActivitiesAbstract = nullptr;
};
}
