// SPDX-FileCopyrightText: 2002 Marc Mutz <mutz@kde.org>
// SPDX-FileCopyrightText: 2024-2026 Laurent Montel <montel@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include "kidentitymanagementwidgets_export.h"
#include <KIdentityManagementCore/Identity>

#include <QComboBox>

namespace KIdentityManagementCore
{
class IdentityManager;
class Identity;
class IdentityActivitiesAbstract;
}
namespace KIdentityManagementWidgets
{
class IdentityComboPrivate;
/*!
 * \class KIdentityManagementWidgets::IdentityCombo
 * \inmodule KIdentityManagementWidgets
 * \inheaderfile KIdentityManagementWidgets/IdentityCombo
 *
 * \brief A combo box that always shows the up-to-date identity list.
 * \author Marc Mutz \<mutz@kde.org\>
 */
class KIDENTITYMANAGEMENTWIDGETS_EXPORT IdentityCombo : public QComboBox
{
    Q_OBJECT
public:
    /*!
     * \brief IdentityCombo constructor
     * \param manager
     * \param parent
     */
    explicit IdentityCombo(KIdentityManagementCore::IdentityManager *manager, QWidget *parent = nullptr);

    ~IdentityCombo() override;

    /*!
     * \brief currentIdentityName
     * \return the current identity name.
     */
    [[nodiscard]] QString currentIdentityName() const;

    /*!
     * \brief currentIdentity
     * \return the current identity id.
     */
    [[nodiscard]] KIdentityManagementCore::Identity::Id currentIdentity() const;

    /*!
     * \brief isDefaultIdentity
     * \return whether the current identity is the default identity.
     */
    [[nodiscard]] bool isDefaultIdentity() const;

    /*!
     * \brief Set the current identity
     * \param identity
     */
    void setCurrentIdentity(const KIdentityManagementCore::Identity &identity);

    /*!
     * \brief Set the current identity by name.
     * \param identityName
     */
    void setCurrentIdentity(const QString &identityName);

    /*!
     * \brief Set the current identity by Id
     * \param uoid
     */
    void setCurrentIdentity(KIdentityManagementCore::Identity::Id uoid);

    /*!
     * \brief Show (default) on the default identity. By default this behavior is disabled.
     * \param showDefault
     */
    void setShowDefault(bool showDefault);

    /*!
     * \brief identityManager
     * \return the IdentityManager used in this combo box
     * \since 4.5
     */
    [[nodiscard]] KIdentityManagementCore::IdentityManager *identityManager() const;

    /*!
     * \brief identityActivitiesAbstract
     * \return
     * \since 6.1
     */
    [[nodiscard]] KIdentityManagementCore::IdentityActivitiesAbstract *identityActivitiesAbstract() const;
    /*!
     * \brief setIdentityActivitiesAbstract
     * \since 6.1
     */
    void setIdentityActivitiesAbstract(KIdentityManagementCore::IdentityActivitiesAbstract *newIdentityActivitiesAbstract);

    /*!
     * \brief enablePlasmaActivities
     * \return
     * \since 6.3
     */
    [[nodiscard]] bool enablePlasmaActivities() const;
    /*!
     * \brief setEnablePlasmaActivities
     * \param newEnablePlasmaActivities
     * \since 6.3
     */
    void setEnablePlasmaActivities(bool newEnablePlasmaActivities);

Q_SIGNALS:

    /*! \brief Emitted whenever the current identity changes
     *
     * This signal is emitted whenever the current identity changes, either
     * by user intervention, on setCurrentIdentity() call, or if the
     * current identity disappears.
     *
     * You might also want to listen to IdentityManager::changed,
     * IdentityManager::deleted and IdentityManager::added.
     *
     * \param uoid the unique object identifier of the new current identity
     */
    void identityChanged(KIdentityManagementCore::Identity::Id uoid);
    /*!
     * \brief identityDeleted
     * \param uoid the unique object identifier of the deleted identity
     */
    void identityDeleted(KIdentityManagementCore::Identity::Id uoid);
    /*!
     * \brief invalidIdentity
     * Emitted when the current identity becomes invalid
     */
    void invalidIdentity();

public Q_SLOTS:
    /*!
     * \brief Connected to IdentityManager::changed()
     * Reloads the list of identities.
     */
    void slotIdentityManagerChanged();

protected Q_SLOTS:
    /*!
     * \brief Internal slot to emit the identity changed signal
     * \param index the index of the combo box that changed
     */
    void slotEmitChanged(int);
    /*!
     * \brief Internal slot to update the tooltip
     * \param uoid the unique object identifier of the identity
     */
    void slotUpdateTooltip(uint uoid);

private:
    std::unique_ptr<IdentityComboPrivate> const d;
};
}
