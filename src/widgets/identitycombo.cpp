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
        : mIdentityModel(new KIdentityManagementCore::IdentityTreeModel(manager, qq))
        , mIdentityProxyModel(new KIdentityManagementCore::IdentityTreeSortProxyModel(qq))
        , q(qq)
    {
        mIdentityProxyModel->setSourceModel(mIdentityModel);
        q->setModel(mIdentityProxyModel);
    }

    KIdentityManagementCore::IdentityTreeModel *const mIdentityModel;
    KIdentityManagementCore::IdentityTreeSortProxyModel *const mIdentityProxyModel;
    IdentityCombo *const q;
};

//@endcond

IdentityCombo::IdentityCombo(IdentityManager *manager, QWidget *parent)
    : QComboBox(parent)
    , d(new KIdentityManagementWidgets::IdentityComboPrivate(manager, this))
{
    connect(manager, &KIdentityManagementCore::IdentityManager::identitiesWereChanged, this, &IdentityCombo::slotIdentityManagerChanged);
    connect(manager, &KIdentityManagementCore::IdentityManager::deleted, this, &IdentityCombo::identityDeleted);
    connect(this, &IdentityCombo::activated, this, &IdentityCombo::slotEmitChanged);
    connect(this, &IdentityCombo::identityChanged, this, &IdentityCombo::slotUpdateTooltip);
    // qDebug() << " d->mIdentityModel " << d->mIdentityModel->rowCount();
    setModelColumn(KIdentityManagementCore::IdentityTreeModel::DisplayIdentityNameRole);
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
    return currentData(KIdentityManagementCore::IdentityTreeModel::IdentityNameRole).toString();
}

uint IdentityCombo::currentIdentity() const
{
    return d->mIdentityProxyModel->mapToSource(d->mIdentityProxyModel->index(currentIndex(), KIdentityManagementCore::IdentityTreeModel::UoidRole))
        .data()
        .toInt();
}

bool IdentityCombo::isDefaultIdentity() const
{
    return currentIdentity() == d->mIdentityModel->identityManager()->defaultIdentity().uoid();
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
    const int idx = d->mIdentityModel->identityManager()->identities().indexOf(name);
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
    const uint oldIdentity = currentIdentity();

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
    setToolTip(d->mIdentityModel->identityManager()->identityForUoid(uoid).fullEmailAddr());
}

IdentityManager *IdentityCombo::identityManager() const
{
    return d->mIdentityModel->identityManager();
}

void IdentityCombo::setShowDefault(bool showDefault)
{
    d->mIdentityModel->setShowDefault(showDefault);
}

#include "moc_identitycombo.cpp"
