/*
  Copyright (c) 2002 Marc Mutz <mutz@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
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

#include <klocalizedstring.h>

#include <assert.h>

using namespace KPIMIdentities;

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class KPIMIdentities::IdentityCombo::Private
{
public:
    Private(IdentityManager *manager, IdentityCombo *qq)
        : mIdentityManager(manager),
          q(qq)
    {

    }
    void reloadCombo();
    void reloadUoidList();

    QList<uint> mUoidList;
    IdentityManager *mIdentityManager;
    IdentityCombo *q;
};

void KPIMIdentities::IdentityCombo::Private::reloadCombo()
{
    const QStringList identities = mIdentityManager->identities();
    // the IM should prevent this from happening:
    assert(!identities.isEmpty());
    q->clear();
    q->addItems(identities);
}

void KPIMIdentities::IdentityCombo::Private::reloadUoidList()
{
    mUoidList.clear();
    IdentityManager::ConstIterator it;
    IdentityManager::ConstIterator end(mIdentityManager->end());
    for (it = mIdentityManager->begin(); it != end; ++it) {
        mUoidList << (*it).uoid();
    }
}

//@endcond

IdentityCombo::IdentityCombo(IdentityManager *manager, QWidget *parent)
    : KComboBox(parent), d(new Private(manager, this))
{
    d->reloadCombo();
    d->reloadUoidList();
    connect(this, SIGNAL(activated(int)), SLOT(slotEmitChanged(int)));
    connect(this, SIGNAL(identityChanged(uint)), this, SLOT(slotUpdateTooltip(uint)));
    connect(manager, SIGNAL(changed()),
            SLOT(slotIdentityManagerChanged()));
    slotUpdateTooltip(currentIdentity());
}

IdentityCombo::~IdentityCombo()
{
    delete d;
}

QString IdentityCombo::currentIdentityName() const
{
    return d->mIdentityManager->identities().at(currentIndex());
}

uint IdentityCombo::currentIdentity() const
{
    return d->mUoidList.at(currentIndex());
}

void IdentityCombo::setCurrentIdentity(const Identity &identity)
{
    setCurrentIdentity(identity.uoid());
}

void IdentityCombo::setCurrentIdentity(const QString &name)
{
    int idx = d->mIdentityManager->identities().indexOf(name);
    if ((idx < 0) || (idx == currentIndex())) {
        return;
    }

    blockSignals(true);    // just in case Qt gets fixed to emit activated() here
    setCurrentIndex(idx);
    blockSignals(false);

    slotEmitChanged(idx);
}

void IdentityCombo::setCurrentIdentity(uint uoid)
{
    int idx = d->mUoidList.indexOf(uoid);
    if ((idx < 0) || (idx == currentIndex())) {
        return;
    }

    blockSignals(true);    // just in case Qt gets fixed to emit activated() here
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
    emit identityChanged(d->mUoidList.at(idx));
}

void IdentityCombo::slotUpdateTooltip(uint uoid)
{
    setToolTip(d->mIdentityManager->identityForUoid(uoid).fullEmailAddr());
}

IdentityManager *IdentityCombo::identityManager() const
{
    return d->mIdentityManager;
}

