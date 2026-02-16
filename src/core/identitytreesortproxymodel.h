// SPDX-FileCopyrightText: 2024-2026 Laurent Montel <montel@kde.org>
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
 *
 * \brief The IdentityTreeSortProxyModel class
 * \author Laurent Montel <montel@kde.org>
 */
class KIDENTITYMANAGEMENTCORE_EXPORT IdentityTreeSortProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    /*!
     * Constructor
     * \param parent the parent object
     */
    explicit IdentityTreeSortProxyModel(QObject *parent);
    /*!
     * Destructor
     */
    ~IdentityTreeSortProxyModel() override;

    /*!
     * Returns the IdentityActivitiesAbstract instance
     * \return the activities handler, or null if not set
     */
    [[nodiscard]] IdentityActivitiesAbstract *identityActivitiesAbstract() const;
    /*!
     * Sets the IdentityActivitiesAbstract instance for filtering
     * \param newIdentityActivitiesAbstract the activities handler to use for filtering
     */
    void setIdentityActivitiesAbstract(IdentityActivitiesAbstract *newIdentityActivitiesAbstract);

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

protected:
    /*!
     * Determines whether a row should be accepted for filtering
     * \param source_row the row in the source model
     * \param source_parent the parent index in the source model
     * \return true if the row should be accepted, false otherwise
     */
    [[nodiscard]] bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    KIDENTITYMANAGEMENTCORE_NO_EXPORT void slotInvalidateFilter();
    IdentityActivitiesAbstract *mIdentityActivitiesAbstract = nullptr;
    bool mEnablePlasmaActivities = false;
};
}
