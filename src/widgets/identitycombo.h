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
/// \brief A combo box that always shows the up-to-date identity list.
/// \author Marc Mutz \<mutz@kde.org\>
class KIDENTITYMANAGEMENTWIDGETS_EXPORT IdentityCombo : public QComboBox
{
    Q_OBJECT
public:
    /// IdentityCombo constructor
    explicit IdentityCombo(KIdentityManagementCore::IdentityManager *manager, QWidget *parent = nullptr);

    ~IdentityCombo() override;

    /// Return the current identity name.
    [[nodiscard]] QString currentIdentityName() const;

    /// Return the current identity id
    [[nodiscard]] KIdentityManagementCore::Identity::Id currentIdentity() const;

    /// Return whether the current identity is the default identity.
    [[nodiscard]] bool isDefaultIdentity() const;

    /// Set the current identity
    void setCurrentIdentity(const KIdentityManagementCore::Identity &identity);

    /// Set the current identity by name.
    void setCurrentIdentity(const QString &identityName);

    /// Set the current identity by Id
    void setCurrentIdentity(KIdentityManagementCore::Identity::Id uoid);

    /// Show (default) on the default identity. By default this behavior is disabled.
    void setShowDefault(bool showDefault);

    /// Returns the IdentityManager used in this combo box.
    /// \since 4.5
    [[nodiscard]] KIdentityManagementCore::IdentityManager *identityManager() const;

    /// \since 6.1
    [[nodiscard]] KIdentityManagementCore::IdentityActivitiesAbstract *identityActivitiesAbstract() const;
    /// \since 6.1
    void setIdentityActivitiesAbstract(KIdentityManagementCore::IdentityActivitiesAbstract *newIdentityActivitiesAbstract);

    /// \since 6.3
    [[nodiscard]] bool enablePlasmaActivities() const;
    /// \since 6.3
    void setEnablePlasmaActivities(bool newEnablePlasmaActivities);

Q_SIGNALS:

    /// \em Really emitted whenever the current identity changes. Either
    /// by user intervention or on setCurrentIdentity() or if the
    /// current identity disappears.
    ///
    /// You might also want to listen to IdentityManager::changed,
    /// IdentityManager::deleted and IdentityManager::added.
    void identityChanged(KIdentityManagementCore::Identity::Id uoid);
    void identityDeleted(KIdentityManagementCore::Identity::Id uoid);
    void invalidIdentity();

public Q_SLOTS:
    /// Connected to IdentityManager::changed(). Reloads the list of identities.
    void slotIdentityManagerChanged();

protected Q_SLOTS:
    void slotEmitChanged(int);
    void slotUpdateTooltip(uint uoid);

private:
    std::unique_ptr<IdentityComboPrivate> const d;
};
}
