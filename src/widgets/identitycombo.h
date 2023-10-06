/*
  SPDX-FileCopyrightText: 2002 Marc Mutz <mutz@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
/**
  @file
  This file is part of the API for handling user identities and defines the
  IdentityCombo class.

  @brief
  A combo box that always shows the up-to-date identity list.

  @author Marc Mutz \<mutz@kde.org\>
 */

#pragma once

#include "kidentitymanagementwidgets_export.h"
#include <QComboBox>

namespace KIdentityManagementCore
{
class IdentityManager;
class Identity;
}
namespace KIdentityManagementWidgets
{
class IdentityComboPrivate;
/**
 * @brief The IdentityCombo class
 * @author Marc Mutz \<mutz@kde.org\>
 */
class KIDENTITYMANAGEMENTWIDGETS_EXPORT IdentityCombo : public QComboBox
{
    Q_OBJECT
public:
    explicit IdentityCombo(KIdentityManagementCore::IdentityManager *manager, QWidget *parent = nullptr);

    ~IdentityCombo() override;
    [[nodiscard]] QString currentIdentityName() const;
    [[nodiscard]] uint currentIdentity() const;
    [[nodiscard]] bool isDefaultIdentity() const;
    void setCurrentIdentity(const QString &identityName);
    void setCurrentIdentity(const KIdentityManagementCore::Identity &identity);
    void setCurrentIdentity(uint uoid);
    /// Show (default) on the default identity. By default this behavior is disabled.
    void setShowDefault(bool showDefault);
    /**
      Returns the IdentityManager used in this combo box.
      @since 4.5
    */
    [[nodiscard]] KIdentityManagementCore::IdentityManager *identityManager() const;

Q_SIGNALS:

    /**
      @em Really emitted whenever the current identity changes. Either
      by user intervention or on setCurrentIdentity() or if the
      current identity disappears.

      You might also want to listen to IdentityManager::changed,
      IdentityManager::deleted and IdentityManager::added.
    */
    void identityChanged(uint uoid);
    void identityDeleted(uint uoid);
    void invalidIdentity();

public Q_SLOTS:
    /**
      Connected to IdentityManager::changed(). Reloads the list of identities.
    */
    void slotIdentityManagerChanged();

protected Q_SLOTS:
    void slotEmitChanged(int);
    void slotUpdateTooltip(uint uoid);

private:
    //@cond PRIVATE
    std::unique_ptr<IdentityComboPrivate> const d;
    //@endcond
};
}
