/*
  SPDX-FileCopyrightText: 2002 Marc Mutz <mutz@kde.org>
  SPDX-FileCopyrightText: 2024 Laurent Montel <montel@kde.org>

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

#include "identitycombo.h"
#include "identity.h"
#include "identitymanager.h"
#include "identitytreemodel.h"
#include "identitytreesortproxymodel.h"

#include <KLocalizedString>

#include <cassert>

using namespace KIdentityManagementWidgets;
using namespace KIdentityManagementCore;
/**
  IdentityComboPrivate class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class KIdentityManagementWidgets::IdentityComboPrivate
{
public:
    IdentityComboPrivate(KIdentityManagementCore::IdentityManager *manager, IdentityCombo *qq)
        : mIdentityManager(manager)
        , q(qq)
    {
    }

    KIdentityManagementCore::IdentityManager *const mIdentityManager;
    KIdentityManagementCore::IdentityTreeModel *mIdentityModel = nullptr;
    KIdentityManagementCore::IdentityTreeSortProxyModel *mIdentityProxyModel = nullptr;
    IdentityCombo *const q;
};

//@endcond

IdentityCombo::IdentityCombo(IdentityManager *manager, QWidget *parent)
    : QComboBox(parent)
    , d(new KIdentityManagementWidgets::IdentityComboPrivate(manager, this))
{
    d->mIdentityModel = new KIdentityManagementCore::IdentityTreeModel(this);
    d->mIdentityProxyModel = new KIdentityManagementCore::IdentityTreeSortProxyModel(this);
    d->mIdentityProxyModel->setSourceModel(d->mIdentityModel);
    connect(manager, &KIdentityManagementCore::IdentityManager::identitiesWereChanged, this, &IdentityCombo::slotIdentityManagerChanged);
    connect(manager, &KIdentityManagementCore::IdentityManager::deleted, this, &IdentityCombo::identityDeleted);
    connect(this, &IdentityCombo::activated, this, &IdentityCombo::slotEmitChanged);
    connect(this, &IdentityCombo::identityChanged, this, &IdentityCombo::slotUpdateTooltip);
    setModel(d->mIdentityProxyModel);
    // qDebug() << " d->mIdentityModel " << d->mIdentityModel->rowCount();
    setModelColumn(KIdentityManagementCore::IdentityTreeModel::IdentityNameRole);
    slotUpdateTooltip(currentIdentity());
}

IdentityCombo::~IdentityCombo() = default;

IdentityActivitiesAbstract *IdentityCombo::identityActivitiesAbstract() const
{
    return d->mIdentityProxyModel->identityActivitiesAbstract();
}

void IdentityCombo::setIdentityActivitiesAbstract(IdentityActivitiesAbstract *newIdentityActivitiesAbstract)
{
    d->mIdentityProxyModel->setIdentityActivitiesAbstract(newIdentityActivitiesAbstract);
}

QString IdentityCombo::currentIdentityName() const
{
    return d->mIdentityManager->identities().at(currentIndex());
}

uint IdentityCombo::currentIdentity() const
{
    return d->mIdentityModel->identityUoid(currentIndex());
}

bool IdentityCombo::isDefaultIdentity() const
{
    return currentIdentity() == d->mIdentityManager->defaultIdentity().uoid();
}

void IdentityCombo::setCurrentIdentity(const Identity &identity)
{
    setCurrentIdentity(identity.uoid());
}

void IdentityCombo::setCurrentIdentity(const QString &name)
{
    if (name.isEmpty()) {
        return;
    }
    const int idx = d->mIdentityManager->identities().indexOf(name);
    if (idx < 0) {
        Q_EMIT invalidIdentity();
        return;
    }

    if (idx == currentIndex()) {
        return;
    }

    blockSignals(true); // just in case Qt gets fixed to emit activated() here
    setCurrentIndex(idx);
    blockSignals(false);

    slotEmitChanged(idx);
}

void IdentityCombo::setCurrentIdentity(uint uoid)
{
    if (uoid == 0) {
        return;
    }
    const int idx = d->mIdentityModel->uoidIndex(uoid);

    if (idx < 0) {
        Q_EMIT invalidIdentity();
        return;
    }
    if (idx == currentIndex()) {
        return;
    }

    blockSignals(true); // just in case Qt gets fixed to emit activated() here
    setCurrentIndex(idx);
    blockSignals(false);

    slotEmitChanged(idx);
}

void IdentityCombo::slotIdentityManagerChanged()
{
    const uint oldIdentity = d->mIdentityModel->identityUoid(currentIndex());

    const int idx = d->mIdentityModel->uoidIndex(oldIdentity);

    blockSignals(true);
    setCurrentIndex(idx < 0 ? 0 : idx);
    blockSignals(false);

    slotUpdateTooltip(currentIdentity());

    if (idx < 0) {
        // apparently our oldIdentity got deleted:
        slotEmitChanged(currentIndex());
    }
}

void IdentityCombo::slotEmitChanged(int idx)
{
    Q_EMIT identityChanged(d->mIdentityModel->identityUoid(idx));
}

void IdentityCombo::slotUpdateTooltip(uint uoid)
{
    setToolTip(d->mIdentityManager->identityForUoid(uoid).fullEmailAddr());
}

IdentityManager *IdentityCombo::identityManager() const
{
    return d->mIdentityManager;
}

void IdentityCombo::setShowDefault(bool showDefault)
{
    d->mIdentityModel->setShowDefault(showDefault);
}

#include "moc_identitycombo.cpp"
