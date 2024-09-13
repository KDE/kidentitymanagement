// SPDX-FileCopyrightText: 2024 Laurent Montel <montel@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#pragma once

#include "kidentitymanagementwidgets_export.h"
#include <QTreeView>
namespace KIdentityManagementCore
{
class IdentityTreeSortProxyModel;
class IdentityActivitiesAbstract;
}
namespace KIdentityManagementWidgets
{
class KIDENTITYMANAGEMENTWIDGETS_EXPORT IdentityTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit IdentityTreeView(QWidget *parent = nullptr);
    ~IdentityTreeView() override;

    void setIdentityActivitiesAbstract(KIdentityManagementCore::IdentityActivitiesAbstract *newIdentityActivitiesAbstract);
    [[nodiscard]] KIdentityManagementCore::IdentityActivitiesAbstract *identityActivitiesAbstract() const;

    [[nodiscard]] bool enablePlasmaActivities() const;
    void setEnablePlasmaActivities(bool newEnablePlasmaActivities);

    [[nodiscard]] KIdentityManagementCore::IdentityTreeSortProxyModel *identityProxyModel() const;

private:
    KIdentityManagementCore::IdentityTreeSortProxyModel *const mIdentityProxyModel;
};
}
