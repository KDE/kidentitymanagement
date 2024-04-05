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
#include "identitytablemodel.h"

#include <KLocalizedString>

#include <cassert>
// #define USE_MODEL 1
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
#ifndef USE_MODEL
    QList<uint> mUoidList;
#endif
    KIdentityManagementCore::IdentityManager *const mIdentityManager;

#ifdef USE_MODEL
    KIdentityManagementWidgets::IdentityTableModel *mIdentityModel = nullptr;
#endif
    IdentityCombo *const q;
#ifndef USE_MODEL
    bool showDefault = false;
#endif
};

void KIdentityManagementWidgets::IdentityComboPrivate::reloadCombo()
{
#ifndef USE_MODEL
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
#endif
}

void KIdentityManagementWidgets::IdentityComboPrivate::reloadUoidList()
{
#ifndef USE_MODEL
    mUoidList.clear();
    KIdentityManagementCore::IdentityManager::ConstIterator it;
    KIdentityManagementCore::IdentityManager::ConstIterator end(mIdentityManager->end());
    for (it = mIdentityManager->begin(); it != end; ++it) {
        mUoidList << (*it).uoid();
    }
#endif
}

//@endcond

IdentityCombo::IdentityCombo(IdentityManager *manager, QWidget *parent)
    : QComboBox(parent)
    , d(new KIdentityManagementWidgets::IdentityComboPrivate(manager, this))
{
#ifdef USE_MODEL
    d->mIdentityModel = new KIdentityManagementWidgets::IdentityTableModel(this);
    connect(manager, &KIdentityManagementCore::IdentityManager::identitiesWereChanged, this, &IdentityCombo::slotIdentityManagerChanged);
    connect(manager, &KIdentityManagementCore::IdentityManager::deleted, this, &IdentityCombo::identityDeleted);
    connect(this, &IdentityCombo::activated, this, &IdentityCombo::slotEmitChanged);
    connect(this, &IdentityCombo::identityChanged, this, &IdentityCombo::slotUpdateTooltip);
    setModel(d->mIdentityModel);
    // qDebug() << " d->mIdentityModel " << d->mIdentityModel->rowCount();
    setModelColumn(KIdentityManagementWidgets::IdentityTableModel::IdentityNameRole);
    slotUpdateTooltip(currentIdentity());
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
#ifdef USE_MODEL
    return d->mIdentityModel->identityUoid(currentIndex());
#else
    return d->mUoidList.at(currentIndex());
#endif
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
#ifdef USE_MODEL
    Q_EMIT d->mIdentityModel->identityUoid(idx);
#else
    Q_EMIT identityChanged(d->mUoidList.at(idx));
#endif
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
#ifdef USE_MODEL
    d->mIdentityModel->setShowDefault(showDefault);
#else
    if (d->showDefault != showDefault) {
        d->showDefault = showDefault;
        d->reloadCombo();
    }
#endif
}

#include "moc_identitycombo.cpp"
