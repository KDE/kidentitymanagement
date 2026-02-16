// SPDX-FileCopyrightText: 2024-2026 Laurent Montel <montel@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#pragma once

#include "kidentitymanagementwidgets_export.h"
#include <QTreeView>
namespace KIdentityManagementCore
{
class IdentityTreeSortProxyModel;
class IdentityActivitiesAbstract;
class Identity;
class IdentityTreeModel;
}
namespace KIdentityManagementWidgets
{
/*!
 * \class KIdentityManagementWidgets::IdentityTreeView
 * \inmodule KIdentityManagementWidgets
 * \inheaderfile KIdentityManagementWidgets/IdentityTreeView
 *
 * \brief The IdentityTreeView class
 */
class KIDENTITYMANAGEMENTWIDGETS_EXPORT IdentityTreeView : public QTreeView
{
    Q_OBJECT
public:
    /*!
     * Constructor
     * \param parent the parent widget
     */
    explicit IdentityTreeView(QWidget *parent = nullptr);
    /*!
     * Destructor
     */
    ~IdentityTreeView() override;

    /*!
     * Sets the IdentityActivitiesAbstract instance for filtering
     * \param newIdentityActivitiesAbstract the activities handler to use for filtering
     */
    void setIdentityActivitiesAbstract(KIdentityManagementCore::IdentityActivitiesAbstract *newIdentityActivitiesAbstract);
    /*!
     * Returns the IdentityActivitiesAbstract instance
     * \return the activities handler, or null if not set
     */
    [[nodiscard]] KIdentityManagementCore::IdentityActivitiesAbstract *identityActivitiesAbstract() const;

    /*!
     * Returns whether Plasma activities support is enabled
     * \return true if Plasma activities filtering is enabled, false otherwise
     */
    [[nodiscard]] bool enablePlasmaActivities() const;
    /*!
     * Sets whether Plasma activities support should be enabled
     * \param newEnablePlasmaActivities true to enable Plasma activities filtering
     */
    void setEnablePlasmaActivities(bool newEnablePlasmaActivities);

    /*!
     * Returns the proxy model for identity filtering and sorting
     * \return the identity proxy model
     */
    [[nodiscard]] KIdentityManagementCore::IdentityTreeSortProxyModel *identityProxyModel() const;

    /*!
     * Returns a modifiable reference to the identity with the given UOID
     * \param uoid the unique object identifier of the identity
     * \return a modifiable reference to the identity
     */
    KIdentityManagementCore::Identity &modifyIdentityForUoid(uint uoid);

    /*!
     * Returns the underlying identity tree model
     * \return the identity tree model
     */
    [[nodiscard]] KIdentityManagementCore::IdentityTreeModel *identityTreeModel() const;

private:
    KIdentityManagementCore::IdentityTreeSortProxyModel *const mIdentityProxyModel;
    KIdentityManagementCore::IdentityTreeModel *mIdentityModel = nullptr;
};
}
