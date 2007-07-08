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

#include <klocale.h>

#include <assert.h>

using namespace KPIMIdentities;

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class KPIMIdentities::IdentityCombo::Private
  {};
//@endcond

IdentityCombo::IdentityCombo( IdentityManager* manager, QWidget * parent )
    : QComboBox( parent ), mIdentityManager( manager ), d( 0 )
{
  reloadCombo();
  reloadUoidList();
  connect( this, SIGNAL( activated( int ) ), SLOT( slotEmitChanged( int ) ) );
  connect( manager, SIGNAL( changed() ),
           SLOT( slotIdentityManagerChanged() ) );
}

IdentityCombo::~IdentityCombo()
{
  delete d;
}

QString IdentityCombo::currentIdentityName() const
{
  return mIdentityManager->identities()[ currentIndex()];
}

uint IdentityCombo::currentIdentity() const
{
  return mUoidList[ currentIndex()];
}

void IdentityCombo::setCurrentIdentity( const Identity & identity )
{
  setCurrentIdentity( identity.uoid() );
}

void IdentityCombo::setCurrentIdentity( const QString & name )
{
  int idx = mIdentityManager->identities().indexOf( name );
  if (( idx < 0 ) || ( idx == currentIndex() ) ) {
    return;
  }

  blockSignals( true );  // just in case Qt gets fixed to emit activated() here
  setCurrentIndex( idx );
  blockSignals( false );

  slotEmitChanged( idx );
}

void IdentityCombo::setCurrentIdentity( uint uoid )
{
  int idx = mUoidList.indexOf( uoid );
  if (( idx < 0 ) || ( idx == currentIndex() ) ) {
    return;
  }

  blockSignals( true );  // just in case Qt gets fixed to emit activated() here
  setCurrentIndex( idx );
  blockSignals( false );

  slotEmitChanged( idx );
}

void IdentityCombo::reloadCombo()
{
  QStringList identities = mIdentityManager->identities();
  // the IM should prevent this from happening:
  assert( !identities.isEmpty() );
  identities.first() = i18n( "%1 (Default)", identities.first() );
  clear();
  addItems( identities );
}

void IdentityCombo::reloadUoidList()
{
  mUoidList.clear();
  IdentityManager::ConstIterator it;
  for ( it = mIdentityManager->begin(); it != mIdentityManager->end(); ++it ) {
    mUoidList << ( *it ).uoid();
  }
}

void IdentityCombo::slotIdentityManagerChanged()
{
  uint oldIdentity = mUoidList[ currentIndex()];

  reloadUoidList();
  int idx = mUoidList.indexOf( oldIdentity );

  blockSignals( true );
  reloadCombo();
  setCurrentIndex( idx < 0 ? 0 : idx );
  blockSignals( false );

  if ( idx < 0 ) {
    // apparently our oldIdentity got deleted:
    slotEmitChanged( currentIndex() );
  }
}

void IdentityCombo::slotEmitChanged( int idx )
{
  emit identityChanged( mIdentityManager->identities()[idx] );
  emit identityChanged( mUoidList[idx] );
}

#include "identitycombo.moc"
