// SPDX-FileCopyrightText: 2024 Laurent Montel <montel@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
#pragma once

#include "kidentitymanagementwidgets_export.h"
#include <QWidget>
namespace KIdentityManagementCore
{
class IdentityActivitiesAbstract;
}
namespace KIdentityManagementWidgets
{
class IdentityTreeView;
class KIDENTITYMANAGEMENTWIDGETS_EXPORT IdentityWidget : public QWidget
{
    Q_OBJECT
public:
    explicit IdentityWidget(QWidget *parent = nullptr);
    ~IdentityWidget() override;

    void setIdentityActivitiesAbstract(KIdentityManagementCore::IdentityActivitiesAbstract *newIdentityActivitiesAbstract);
    [[nodiscard]] KIdentityManagementCore::IdentityActivitiesAbstract *identityActivitiesAbstract() const;

private:
    KIDENTITYMANAGEMENTWIDGETS_NO_EXPORT void slotCustomContextMenuRequested(const QPoint &pos);
    KIDENTITYMANAGEMENTWIDGETS_NO_EXPORT void slotNewIdentity();
    KIDENTITYMANAGEMENTWIDGETS_NO_EXPORT void updateButtons();
    IdentityTreeView *const mIdentityTreeView;
};

}
