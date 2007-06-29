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

#ifndef _KPIM_IDENTITYCOMBO_H_
#define _KPIM_IDENTITYCOMBO_H_

#include <QComboBox>
#include <kpimidentities/libkpimidentities_export.h>

class QString;

namespace KPIMIdentities {

class IdentityManager;
class Identity;

/**
 * @short A combo box that always shows the up-to-date identity list.
 * @author Marc Mutz <mutz@kde.org>
 **/

class KPIMIDENTITIES_EXPORT IdentityCombo : public QComboBox {
  Q_OBJECT
public:
  explicit IdentityCombo( IdentityManager* manager, QWidget * parent=0 );

  QString currentIdentityName() const;
  uint currentIdentity() const;
  void setCurrentIdentity( const QString & identityName );
  void setCurrentIdentity( const Identity & identity );
  void setCurrentIdentity( uint uoid );

signals:
  /** @deprecated
   *  @em Really emitted whenever the current identity changes. Either
   *  by user intervention or on  setCurrentIdentity() or if the
   *  current identity disappears.
   **/
  void identityChanged( const QString & identityName );

  /** @em Really emitted whenever the current identity changes. Either
   *  by user intervention or on setCurrentIdentity() or if the
   *  current identity disappears.
   *
   *  You might also want to listen to IdentityManager::changed,
   *  IdentityManager::deleted and IdentityManager::added.
   **/
  void identityChanged( uint uoid );

public slots:
  /** Connected to IdentityManager::changed(). Reloads the list
   *  of identities.
   **/
  void slotIdentityManagerChanged();

protected slots:
  void slotEmitChanged(int);

protected:
  void reloadCombo();
  void reloadUoidList();

protected:
  QList<uint> mUoidList;
  IdentityManager* mIdentityManager;
};

} // namespace

#endif // _KPIM_IDENTITYCOMBO_H_
