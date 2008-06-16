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

// config keys:
static const char configKeyDefaultIdentity[] = "Default Identity";

#include "identitymanager.h"
#include "identity.h" // for IdentityList::{export,import}Data

#include <kpimutils/email.h> // for static helper functions

#include <kemailsettings.h> // for IdentityEntry::fromControlCenter()
#include <klocale.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kuser.h>
#include <kconfiggroup.h>

#include <QList>
#include <QRegExp>
#include <QtDBus/QtDBus>

#include <assert.h>
#include <krandom.h>

#include "identitymanageradaptor.h"

using namespace KPIMIdentities;

static QString newDBusObjectName()
{
  static int s_count = 0;
  QString name( "/KPIMIDENTITIES_IdentityManager" );
  if ( s_count++ ) {
    name += '_';
    name += QString::number( s_count );
  }
  return name;
}


/**
 *   Private class that helps to provide binary compatibility between releases.
 *   @internal
 */
//@cond PRIVATE
class KPIMIdentities::IdentityManager::Private
{
  public:
    Private( KPIMIdentities::IdentityManager* );
    void writeConfig() const;
    void readConfig( KConfig *config );
    void createDefaultIdentity();
    QStringList groupList( KConfig *config ) const;
    void slotIdentitiesChanged( const QString &id );
    KConfig *mConfig;
      
    QList<Identity> mIdentities;
    QList<Identity> mShadowIdentities;

    // returns a new Unique Object Identifier
    int newUoid();

    bool readOnly;
    KPIMIdentities::IdentityManager *q;
};
//@endcond

IdentityManager::Private::Private( KPIMIdentities::IdentityManager *manager): q( manager )
{
}

void IdentityManager::Private::writeConfig() const
{
  QStringList identities = groupList( mConfig );
  for ( QStringList::Iterator group = identities.begin();
        group != identities.end(); ++group )
    mConfig->deleteGroup( *group );
  int i = 0;
  for ( ConstIterator it = mIdentities.begin();
        it != mIdentities.end(); ++it, ++i ) {
    KConfigGroup cg( mConfig, QString::fromLatin1( "Identity #%1" ).arg( i ) );
    (*it).writeConfig( cg );
    if ( (*it).isDefault() ) {
      // remember which one is default:
      KConfigGroup general( mConfig, "General" );
      general.writeEntry( configKeyDefaultIdentity, (*it).uoid() );

      // Also write the default identity to emailsettings
      KEMailSettings es;
      es.setSetting( KEMailSettings::RealName, (*it).fullName() );
      es.setSetting( KEMailSettings::EmailAddress, (*it).emailAddr() );
      es.setSetting( KEMailSettings::Organization, (*it).organization() );
      es.setSetting( KEMailSettings::ReplyToAddress, (*it).replyToAddr() );
    }
  }
  mConfig->sync();
}

void IdentityManager::Private::readConfig( KConfig *config )
{
  mIdentities.clear();

  QStringList identities = groupList( config );
  if ( identities.isEmpty() ) {
    return; // nothing to be done...
  }

  KConfigGroup general( config, "General" );
  uint defaultIdentity = general.readEntry( configKeyDefaultIdentity, 0 );
  bool haveDefault = false;

  for ( QStringList::Iterator group = identities.begin();
        group != identities.end(); ++group ) {
    KConfigGroup configGroup( config, *group );
    Identity identity;
    identity.readConfig( configGroup );
    if ( !haveDefault && identity.uoid() == defaultIdentity ) {
      haveDefault = true;
      identity.setIsDefault( true );
    }
    mIdentities << identity;
  }
  if ( !haveDefault ) {
    kWarning( 5325 ) << "IdentityManager: There was no default identity."
                     << "Marking first one as default.";
    mIdentities.first().setIsDefault( true );
  }
  qSort( mIdentities );

  mShadowIdentities = mIdentities;
}

void IdentityManager::Private::createDefaultIdentity()
{
  QString fullName, emailAddress;
  bool done = false;

  // Check if the application has any settings
  q->createDefaultIdentity( fullName, emailAddress );

  // If not, then use the kcontrol settings
  if ( fullName.isEmpty() && emailAddress.isEmpty() ) {
    KEMailSettings emailSettings;
    fullName = emailSettings.getSetting( KEMailSettings::RealName );
    emailAddress = emailSettings.getSetting( KEMailSettings::EmailAddress );

    if ( !fullName.isEmpty() && !emailAddress.isEmpty() ) {
      q->newFromControlCenter( i18nc( "use default address from control center",
                                   "Default" ) );
      done = true;
    } else {
      // If KEmailSettings doesn't have name and address, generate something from KUser
      KUser user;
      if ( fullName.isEmpty() ) {
        fullName = user.property( KUser::FullName ).toString();
      }
      if ( emailAddress.isEmpty() ) {
        emailAddress = user.loginName();
        if ( !emailAddress.isEmpty() ) {
          KConfigGroup general( mConfig, "General" );
          QString defaultdomain = general.readEntry( "Default domain" );
          if ( !defaultdomain.isEmpty() ) {
            emailAddress += '@' + defaultdomain;
          } else {
            emailAddress.clear();
          }
        }
      }
    }
  }

  if ( !done ) {
    mShadowIdentities << Identity( i18nc( "show default identity", "Default" ), fullName, emailAddress );
  }

  mShadowIdentities.last().setIsDefault( true );
  mShadowIdentities.last().setUoid( newUoid() );
  if ( readOnly ) { // commit won't do it in readonly mode
    mIdentities = mShadowIdentities;
  }
}

QStringList IdentityManager::Private::groupList( KConfig *config ) const
{
  return config->groupList().filter( QRegExp( "^Identity #\\d+$" ) );
}

int IdentityManager::Private::newUoid()
{
  int uoid;

  // determine the UOIDs of all saved identities
  QList<uint> usedUOIDs;
  for ( QList<Identity>::ConstIterator it = mIdentities.begin();
        it != mIdentities.end(); ++it )
    usedUOIDs << (*it).uoid();

  if ( q->hasPendingChanges() ) {
    // add UOIDs of all shadow identities. Yes, we will add a lot of duplicate
    // UOIDs, but avoiding duplicate UOIDs isn't worth the effort.
    for ( QList<Identity>::ConstIterator it = mShadowIdentities.begin();
          it != mShadowIdentities.end(); ++it ) {
      usedUOIDs << (*it).uoid();
    }
  }

  usedUOIDs << 0; // no UOID must be 0 because this value always refers to the
  // default identity

  do {
    uoid = KRandom::random();
  } while ( usedUOIDs.indexOf( uoid ) != -1 );

  return uoid;
}

void IdentityManager::Private::slotIdentitiesChanged( const QString &id )
{
  kDebug( 5325 ) <<" KPIMIdentities::IdentityManager::slotIdentitiesChanged :" << id;
  if ( id != QDBusConnection::sessionBus().baseService() ) {
    mConfig->reparseConfiguration();
    Q_ASSERT( !q->hasPendingChanges() );
    readConfig( mConfig );
    emit q->changed();
  }
}

// --- non private implementation -------

IdentityManager::IdentityManager( bool readonly, QObject *parent,
                                  const char *name )
    : QObject( parent ), d( new Private( this ) )
{
  setObjectName( name );
  new IdentityManagerAdaptor( this );
  QDBusConnection dbus = QDBusConnection::sessionBus();
  const QString dbusPath = newDBusObjectName();
  const QString dbusInterface = "org.kde.pim.IdentityManager";
  dbus.registerObject( dbusPath, this );
  dbus.connect( QString(), dbusPath, dbusInterface, "identitiesChanged", this,
                SLOT( slotIdentitiesChanged( QString ) ) );

  d->mConfig = new KConfig( "emailidentities" );
  d->readOnly = readonly;
  d->readConfig( d->mConfig );
  if ( d->mIdentities.isEmpty() ) {
    kDebug( 5325 ) << "emailidentities is empty -> convert from kmailrc";
    // No emailidentities file, or an empty one due to broken conversion
    // (kconf_update bug in kdelibs <= 3.2.2)
    // => convert it, i.e. read settings from kmailrc
    KConfig kmailConf( "kmailrc" );
    d->readConfig( &kmailConf );
  }
  // we need at least a default identity:
  if ( d->mIdentities.isEmpty() ) {
    kDebug( 5325 ) << "IdentityManager: No identity found. Creating default.";
    d->createDefaultIdentity();
    commit();
  }
  // Migration: people without settings in kemailsettings should get some
  if ( KEMailSettings().getSetting( KEMailSettings::EmailAddress ).isEmpty() ) {
    d->writeConfig();
  }
}

IdentityManager::~IdentityManager()
{
  kWarning( hasPendingChanges(), 5325 )
  << "IdentityManager: There were uncommitted changes!";
  delete d;
}

void IdentityManager::commit()
{
  // early out:
  if ( !hasPendingChanges() || d->readOnly ) {
    return;
  }

  QList<uint> seenUOIDs;
  for ( QList<Identity>::ConstIterator it = d->mIdentities.begin();
        it != d->mIdentities.end(); ++it ) {
    seenUOIDs << (*it).uoid();
  }

  QList<uint> changedUOIDs;
  // find added and changed identities:
  for ( QList<Identity>::ConstIterator it = d->mShadowIdentities.begin();
        it != d->mShadowIdentities.end(); ++it ) {
    int index = seenUOIDs.indexOf( (*it).uoid() );
    if ( index != -1 ) {
      uint uoid = seenUOIDs.at( index );
      const Identity &orig = identityForUoid( uoid );  // look up in mIdentities
      if ( *it != orig ) {
        // changed identity
        kDebug( 5325 ) << "emitting changed() for identity" << uoid;
        emit changed(*it);
        changedUOIDs << uoid;
      }
      seenUOIDs.removeAll( uoid );
    } else {
      // new identity
      kDebug( 5325 ) << "emitting added() for identity" << (*it).uoid();
      emit added(*it);
    }
  }

  // what's left are deleted identities:
  for ( QList<uint>::ConstIterator it = seenUOIDs.begin();
        it != seenUOIDs.end(); ++it ) {
    kDebug( 5325 ) << "emitting deleted() for identity" << (*it);
    emit deleted(*it);
  }

  d->mIdentities = d->mShadowIdentities;
  d->writeConfig();

  // now that mIdentities has all the new info, we can emit the added/changed
  // signals that ship a uoid. This is because the slots might use
  // identityForUoid(uoid)...
  for ( QList<uint>::ConstIterator it = changedUOIDs.begin();
        it != changedUOIDs.end(); ++it )
    emit changed(*it);

  emit changed(); // normal signal

  // DBus signal for other IdentityManager instances
  emit identitiesChanged( QDBusConnection::sessionBus().baseService() );
}

void IdentityManager::rollback()
{
  d->mShadowIdentities = d->mIdentities;
}

bool IdentityManager::hasPendingChanges() const
{
  return d->mIdentities != d->mShadowIdentities;
}

QStringList IdentityManager::identities() const
{
  QStringList result;
  for ( ConstIterator it = d->mIdentities.begin();
        it != d->mIdentities.end(); ++it )
    result << (*it).identityName();
  return result;
}

QStringList IdentityManager::shadowIdentities() const
{
  QStringList result;
  for ( ConstIterator it = d->mShadowIdentities.begin();
        it != d->mShadowIdentities.end(); ++it )
    result << (*it).identityName();
  return result;
}

void IdentityManager::sort()
{
  qSort( d->mShadowIdentities );
}


IdentityManager::ConstIterator IdentityManager::begin() const
{
  return d->mIdentities.begin();
}

IdentityManager::ConstIterator IdentityManager::end() const
{
  return d->mIdentities.end();
}

IdentityManager::Iterator IdentityManager::modifyBegin()
{
  return d->mShadowIdentities.begin();
}

IdentityManager::Iterator IdentityManager::modifyEnd()
{
  return d->mShadowIdentities.end();
}

const Identity &IdentityManager::identityForUoid( uint uoid ) const
{
  for ( ConstIterator it = begin(); it != end(); ++it ) {
    if ( (*it).uoid() == uoid ) {
      return (*it);
    }
  }
  return Identity::null();
}

const Identity &IdentityManager::identityForUoidOrDefault( uint uoid ) const
{
  const Identity &ident = identityForUoid( uoid );
  if ( ident.isNull() ) {
    return defaultIdentity();
  } else {
    return ident;
  }
}

const Identity &IdentityManager::identityForAddress(
  const QString &addresses ) const
{
  QStringList addressList = KPIMUtils::splitAddressList( addresses );
  for ( ConstIterator it = begin(); it != end(); ++it ) {
    for ( QStringList::ConstIterator addrIt = addressList.begin();
          addrIt != addressList.end(); ++addrIt ) {
      if ( (*it).emailAddr().toLower() == KPIMUtils::extractEmailAddress( *addrIt ).toLower() ) {
        return (*it);
      }
    }
  }
  return Identity::null();
}

bool IdentityManager::thatIsMe( const QString &addressList ) const
{
  return !identityForAddress( addressList ).isNull();
}

Identity &IdentityManager::modifyIdentityForName( const QString &name )
{
  for ( Iterator it = modifyBegin(); it != modifyEnd(); ++it ) {
    if ( (*it).identityName() == name ) {
      return (*it);
    }
  }

  kWarning( 5325 ) << "IdentityManager::modifyIdentityForName() used as"
                   << "newFromScratch() replacement!"
                   << endl << "  name == \"" << name << "\"";
  return newFromScratch( name );
}

Identity &IdentityManager::modifyIdentityForUoid( uint uoid )
{
  for ( Iterator it = modifyBegin(); it != modifyEnd(); ++it ) {
    if ( (*it).uoid() == uoid ) {
      return (*it);
    }
  }

  kWarning( 5325 ) << "IdentityManager::identityForUoid() used as"
                   << "newFromScratch() replacement!"
                   << endl << "  uoid == \"" << uoid << "\"";
  return newFromScratch( i18n( "Unnamed" ) );
}

const Identity &IdentityManager::defaultIdentity() const
{
  for ( ConstIterator it = begin(); it != end(); ++it ) {
    if ( (*it).isDefault() ) {
      return (*it);
    }
  }

  if ( d->mIdentities.isEmpty() )
      kFatal( 5325 ) << "IdentityManager: No default identity found!";
  else
      kWarning( 5325 ) << "IdentityManager: No default identity found!";
  return *begin();
}

bool IdentityManager::setAsDefault( uint uoid )
{
  // First, check if the identity actually exists:
  bool found = false;
  for ( ConstIterator it = d->mShadowIdentities.begin();
        it != d->mShadowIdentities.end(); ++it )
    if ( (*it).uoid() == uoid ) {
      found = true;
      break;
    }

  if ( !found ) {
    return false;
  }

  // Then, change the default as requested:
  for ( Iterator it = modifyBegin(); it != modifyEnd(); ++it ) {
    (*it).setIsDefault( (*it).uoid() == uoid );
  }

  // and re-sort:
  sort();
  return true;
}

bool IdentityManager::removeIdentity( const QString &name )
{
  if ( d->mShadowIdentities.size() <= 1 )
    return false;

  for ( Iterator it = modifyBegin(); it != modifyEnd(); ++it ) {
    if ( (*it).identityName() == name ) {
      bool removedWasDefault = (*it).isDefault();
      d->mShadowIdentities.erase( it );
      if ( removedWasDefault ) {
        d->mShadowIdentities.first().setIsDefault( true );
      }
      return true;
    }
  }
  return false;
}

Identity &IdentityManager::newFromScratch( const QString &name )
{
  return newFromExisting( Identity( name ) );
}

Identity &IdentityManager::newFromControlCenter( const QString &name )
{
  KEMailSettings es;
  es.setProfile( es.defaultProfileName() );

  return
    newFromExisting( Identity( name,
                               es.getSetting( KEMailSettings::RealName ),
                               es.getSetting( KEMailSettings::EmailAddress ),
                               es.getSetting( KEMailSettings::Organization ),
                               es.getSetting( KEMailSettings::ReplyToAddress ) ) );
}

Identity &IdentityManager::newFromExisting( const Identity &other,
    const QString &name )
{
  d->mShadowIdentities << other;
  Identity &result = d->mShadowIdentities.last();
  result.setIsDefault( false );  // we don't want two default identities!
  result.setUoid( d->newUoid() );  // we don't want two identies w/ same UOID
  if ( !name.isNull() ) {
    result.setIdentityName( name );
  }
  return result;
}


QStringList KPIMIdentities::IdentityManager::allEmails() const
{
  QStringList lst;
  for ( ConstIterator it = begin(); it != end(); ++it ) {
    lst << (*it).emailAddr();
  }
  return lst;
}

void KPIMIdentities::IdentityManager::slotRollback()
{
  rollback();
}

#include "identitymanager.moc"
