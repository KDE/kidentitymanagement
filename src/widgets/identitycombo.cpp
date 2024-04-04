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
#include "identitymodel.h"

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

    void reloadCombo();
    void reloadUoidList();

    QList<uint> mUoidList;
    KIdentityManagementCore::IdentityManager *const mIdentityManager;
    KIdentityManagementCore::IdentityModel *mIdentityModel = nullptr;
    IdentityCombo *const q;
    bool showDefault = false;
};

void KIdentityManagementWidgets::IdentityComboPrivate::reloadCombo()
{
    QStringList identities;
    identities.reserve(mIdentityManager->identities().count());
    KIdentityManagementCore::IdentityManager::ConstIterator it;
    KIdentityManagementCore::IdentityManager::ConstIterator end(mIdentityManager->end());
    for (it = mIdentityManager->begin(); it != end; ++it) {
        if (showDefault && it->isDefault()) {
            identities << QString(it->identityName() + i18nc("Default identity", " (default)"));
        } else {
            identities << it->identityName();
        }
    }
    // the IM should prevent this from happening:
    assert(!identities.isEmpty());
    q->clear();
    q->addItems(identities);
}

void KIdentityManagementWidgets::IdentityComboPrivate::reloadUoidList()
{
    mUoidList.clear();
    KIdentityManagementCore::IdentityManager::ConstIterator it;
    KIdentityManagementCore::IdentityManager::ConstIterator end(mIdentityManager->end());
    for (it = mIdentityManager->begin(); it != end; ++it) {
        mUoidList << (*it).uoid();
    }
}

//@endcond

IdentityCombo::IdentityCombo(IdentityManager *manager, QWidget *parent)
    : QComboBox(parent)
    , d(new KIdentityManagementWidgets::IdentityComboPrivate(manager, this))
{
#if 0
    // TODO use IdentityModel
    d->mIdentityModel = new KIdentityManagementCore::IdentityModel(this);
    setModel(d->mIdentityModel);
    // qDebug() << " d->mIdentityModel " << d->mIdentityModel->rowCount();
    setModelColumn(KIdentityManagementCore::IdentityModel::IdentityNameRole);
#else
    d->reloadCombo();
    d->reloadUoidList();
    connect(this, &IdentityCombo::activated, this, &IdentityCombo::slotEmitChanged);
    connect(this, &IdentityCombo::identityChanged, this, &IdentityCombo::slotUpdateTooltip);
    connect(manager, &KIdentityManagementCore::IdentityManager::identitiesWereChanged, this, &IdentityCombo::slotIdentityManagerChanged);
    connect(manager, &KIdentityManagementCore::IdentityManager::deleted, this, &IdentityCombo::identityDeleted);
    slotUpdateTooltip(currentIdentity());
#endif
}

IdentityCombo::~IdentityCombo() = default;

QString IdentityCombo::currentIdentityName() const
{
    return d->mIdentityManager->identities().at(currentIndex());
}

uint IdentityCombo::currentIdentity() const
{
    return d->mUoidList.at(currentIndex());
}

bool IdentityCombo::isDefaultIdentity() const
{
    return d->mUoidList.at(currentIndex()) == d->mIdentityManager->defaultIdentity().uoid();
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
    const int idx = d->mUoidList.indexOf(uoid);
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
    uint oldIdentity = d->mUoidList.at(currentIndex());

    d->reloadUoidList();
    int idx = d->mUoidList.indexOf(oldIdentity);

    blockSignals(true);
    d->reloadCombo();
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
    Q_EMIT identityChanged(d->mUoidList.at(idx));
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
    if (d->showDefault != showDefault) {
        d->showDefault = showDefault;
        d->reloadCombo();
    }
}

#include "moc_identitycombo.cpp"
