/*
    This file is part of libkresources.
    
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>
    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <dcopclient.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kstandarddirs.h>

#include "resource.h"
#include "factory.h"
#include "manager.h"
#include "managerimpl.h"
#include "manageriface_stub.h"

using namespace KRES;

ManagerImpl::ManagerImpl( const QString &family )
  : DCOPObject( "ManagerIface_" + family.utf8() + "_" +
                QCString().setNum( kapp->random() ) ),
    mFamily( family ), mConfig( 0 ), mStdConfig( 0 ), mStandard( 0 ),
    mFactory( 0 )
{
  kdDebug(5650) << "ManagerImpl::ManagerImpl()" << endl;

  // Register with DCOP
  if ( !kapp->dcopClient()->isRegistered() ) {
    kapp->dcopClient()->registerAs( "Manager" );
    kapp->dcopClient()->setDefaultObject( objId() );
  }

  kdDebug(5650) << "Connecting DCOP signals..." << endl;
  if ( ! connectDCOPSignal( 0, "ManagerIface_" + family.utf8() + "_*", "signalResourceAdded(QString)", "dcopResourceAdded(QString)", false ) )
    kdWarning() << "Could not connect ResourceAdded signal!" << endl;

  // "ManagerIface_" + mFamily.utf8()
  if ( ! connectDCOPSignal( 0, "ManagerIface_" + family.utf8() + "_*", "signalResourceModified(QString)", "dcopResourceModified(QString)", false ) )
    kdWarning() << "Could not connect ResourceModified signal!" << endl;

  if ( ! connectDCOPSignal( 0, "ManagerIface_" + family.utf8() + "_*", "signalResourceDeleted(QString)", "dcopResourceDeleted(QString)", false ) )
    kdWarning() << "Could not connect ResourceDeleted signal!" << endl;

  kapp->dcopClient()->setNotifications( true );
}

ManagerImpl::~ManagerImpl()
{
  kdDebug(5650) << "ManagerImpl::~ManagerImpl()" << endl;

  Resource::List::ConstIterator it;
  for ( it = mResources.begin(); it != mResources.end(); ++it ) {
    delete *it;
  }
 
  delete mStdConfig;
}

void ManagerImpl::createStandardConfig()
{
  if ( !mStdConfig ) {
    QString file = defaultConfigFile( mFamily );
    mStdConfig = new KConfig( file );
  }
  
  mConfig = mStdConfig;
}

void ManagerImpl::readConfig( KConfig *cfg )
{
  kdDebug(5650) << "ManagerImpl::readConfig()" << endl;

  delete mFactory;
  mFactory = Factory::self( mFamily );

  if ( !cfg ) {
    createStandardConfig();
  } else {
    mConfig = cfg;
  }

  mStandard = 0;

  mConfig->setGroup( "General" );

  QStringList keys = mConfig->readListEntry( "ResourceKeys" );
  keys += mConfig->readListEntry( "PassiveResourceKeys" );

  QString standardKey = mConfig->readEntry( "Standard" );

  for ( QStringList::Iterator it = keys.begin(); it != keys.end(); ++it ) {
    readResourceConfig( *it, false );
  }
}

void ManagerImpl::writeConfig( KConfig *cfg )
{
  kdDebug(5650) << "ManagerImpl::writeConfig()" << endl;

  if ( !cfg ) {
    createStandardConfig();
  } else {
    mConfig = cfg;
  }

  QStringList activeKeys;
  QStringList passiveKeys;

  // First write all keys, collect active and passive keys on the way
  Resource::List::Iterator it;
  for ( it = mResources.begin(); it != mResources.end(); ++it ) {
    writeResourceConfig( *it, false );

    QString key = (*it)->identifier();
    if( (*it)->isActive() )
      activeKeys.append( key );
    else
      passiveKeys.append( key );
  }

  // And then the general group

  kdDebug(5650) << "Saving general info" << endl;
  mConfig->setGroup( "General" );
  mConfig->writeEntry( "ResourceKeys", activeKeys );
  mConfig->writeEntry( "PassiveResourceKeys", passiveKeys );
  if ( mStandard ) 
    mConfig->writeEntry( "Standard", mStandard->identifier() );
  else
    mConfig->writeEntry( "Standard", "" );

  mConfig->sync();
  kdDebug(5650) << "ManagerImpl::save() finished" << endl;
}

void ManagerImpl::add( Resource *resource, bool useDCOP )
{
  resource->setActive( true );

  if ( mResources.isEmpty() ) {
    mStandard = resource;
  }

  mResources.append( resource );

  writeResourceConfig( resource, true );

  if ( useDCOP ) signalResourceAdded( resource->identifier() );
}

void ManagerImpl::remove( Resource *resource, bool useDCOP )
{
  if ( mStandard == resource ) mStandard = 0;
  removeResource( resource );

  mResources.remove( resource );

  if ( useDCOP ) signalResourceDeleted( resource->identifier() );

  delete resource;

  kdDebug(5650) << "Finished ManagerImpl::remove()" << endl;
}

void ManagerImpl::setActive( Resource *resource, bool active )
{
  if ( resource && resource->isActive() != active ) {
    resource->setActive( active );
  }
}

Resource *ManagerImpl::standardResource() 
{
  return mStandard;
}

void ManagerImpl::setStandardResource( Resource *resource ) 
{
  mStandard = resource;
}

void ManagerImpl::resourceChanged( Resource *resource )
{
  writeResourceConfig( resource, true );

  signalResourceModified( resource->identifier() );
//  ManagerIface_stub allManagers( "*", "ManagerIface_" + mFamily.utf8() );
//  allManagers.dcopResourceModified( resource->identifier() );
}

// DCOP asynchronous functions

void ManagerImpl::dcopResourceAdded( QString identifier )
{
  if ( kapp->dcopClient()->senderId() == kapp->dcopClient()->appId() ) {
    kdDebug(5650) << "Ignore DCOP call since am self source" << endl;
    return;
  }
  kdDebug(5650) << "Receive DCOP call: added resource " << identifier << endl;

  if ( getResource( identifier ) ) {
    kdDebug(5650) << "Wait a minute! This resource is already known to me!" << endl;
  }

  if ( !mConfig ) createStandardConfig();

  mConfig->reparseConfiguration();
  Resource *resource = readResourceConfig( identifier, true );

  if ( resource ) {
    if ( mListener ) {
      kdDebug(5650) << "Notifying Listener" << endl;
      mListener->resourceAdded( resource );
    }
  }
  else 
    kdError() << "Received DCOP: resource added for unknown resource " << identifier << endl;
}

void ManagerImpl::dcopResourceModified( QString identifier )
{
  if ( kapp->dcopClient()->senderId() == kapp->dcopClient()->appId() ) {
    kdDebug(5650) << "Ignore DCOP call since am self source" << endl;
    return;
  }
  kdDebug(5650) << "Receive DCOP call: modified resource " << identifier << endl;

  Resource *resource = getResource( identifier );
  if ( resource ) {
    if ( mListener ) {
      kdDebug(5650) << "Notifying Listener" << endl;
      mListener->resourceModified( resource );
    }
  } else 
    kdError() << "Received DCOP: resource modified for unknown resource " << identifier << endl;
}

void ManagerImpl::dcopResourceDeleted( QString identifier )
{
  if ( kapp->dcopClient()->senderId() == kapp->dcopClient()->appId() ) {
    kdDebug(5650) << "Ignore DCOP call since am self source" << endl;
    return;
  }
  kdDebug(5650) << "Receive DCOP call: deleted resource " << identifier << endl;

  Resource *resource = getResource( identifier );
  if ( resource ) {
    if ( mListener ) {
      kdDebug(5650) << "Notifying Listener" << endl;
      mListener->resourceDeleted( resource );
    }

    kdDebug(5650) << "Removing item from mResources" << endl;
    // Now delete item
    if ( mStandard == resource )
      mStandard = 0;
    mResources.remove( resource );
  } else
    kdError() << "Received DCOP: resource deleted for unknown resource " << identifier << endl;

}

QStringList ManagerImpl::resourceNames()
{
  QStringList result;

  Resource::List::ConstIterator it;
  for ( it = mResources.begin(); it != mResources.end(); ++it ) {
    result.append( (*it)->resourceName() );
  }
  return result;
}

Resource::List *ManagerImpl::resourceList()
{
  return &mResources;
}

QPtrList<Resource> ManagerImpl::resources()
{
  QPtrList<Resource> result;

  Resource::List::ConstIterator it;
  for ( it = mResources.begin(); it != mResources.end(); ++it ) {
    result.append( *it );
  }
  return result;
}

QPtrList<Resource> ManagerImpl::resources( bool active )
{
  QPtrList<Resource> result;

  Resource::List::ConstIterator it;
  for ( it = mResources.begin(); it != mResources.end(); ++it ) {
    if ( (*it)->isActive() == active ) {
      result.append( *it );
    }
  }
  return result;
}

void ManagerImpl::setListener( ManagerImplListener *listener )
{
  mListener = listener;
}

Resource* ManagerImpl::readResourceConfig( const QString& identifier,
                                                   bool checkActive )
{
  kdDebug() << "ManagerImpl::readResourceConfig() " << identifier << endl;

  mConfig->setGroup( "Resource_" + identifier );

  QString type = mConfig->readEntry( "ResourceType" );
  QString name = mConfig->readEntry( "ResourceName" );
  Resource *resource = mFactory->resource( type, mConfig );
  if ( !resource ) {
    kdDebug(5650) << "Failed to create resource with id " << identifier << endl;
    return 0;
  }

  if ( resource->identifier().isEmpty() )
    resource->setIdentifier( identifier );

  mConfig->setGroup( "General" );

  QString standardKey = mConfig->readEntry( "Standard" );
  if ( standardKey == identifier ) {
    mStandard = resource;
  }

  if ( checkActive ) {
    QStringList activeKeys = mConfig->readListEntry( "ResourceKeys" );
    resource->setActive( activeKeys.contains( identifier ) );
  }
  mResources.append( resource );

  return resource;
}

void ManagerImpl::writeResourceConfig( Resource *resource,
                                               bool checkActive )
{
  QString key = resource->identifier();

  kdDebug(5650) << "Saving resource " << key << endl;

  if ( !mConfig ) createStandardConfig();

  mConfig->setGroup( "Resource_" + key );
  resource->writeConfig( mConfig );

  mConfig->setGroup( "General" );
  QString standardKey = mConfig->readEntry( "Standard" );

  if ( resource == mStandard  && standardKey != key )
    mConfig->writeEntry( "Standard", resource->identifier() );
  else if ( resource != mStandard && standardKey == key )
    mConfig->writeEntry( "Standard", "" );
  
  if ( checkActive ) {
    QStringList activeKeys = mConfig->readListEntry( "ResourceKeys" );
    if ( resource->isActive() && !activeKeys.contains( key ) ) {
      activeKeys.append( resource->identifier() );
      mConfig->writeEntry( "ResourceKeys", activeKeys );
    } else if ( !resource->isActive() && activeKeys.contains( key ) ) {
      activeKeys.remove( key );
      mConfig->writeEntry( "ResourceKeys", activeKeys );
    }
  }

  mConfig->sync();
}

void ManagerImpl::removeResource( Resource *resource )
{
  QString key = resource->identifier();

  if ( !mConfig ) createStandardConfig();
  
  mConfig->setGroup( "General" );
  QStringList activeKeys = mConfig->readListEntry( "ResourceKeys" );
  if ( activeKeys.contains( key ) ) {
    activeKeys.remove( key );
    mConfig->writeEntry( "ResourceKeys", activeKeys );
  } else {
    QStringList passiveKeys = mConfig->readListEntry( "PassiveResourceKeys" );
    passiveKeys.remove( key );
    mConfig->writeEntry( "PassiveResourceKeys", passiveKeys );
  }

  QString standardKey = mConfig->readEntry( "Standard" );
  if ( standardKey == key ) {
    mConfig->writeEntry( "Standard", "" );
  }

  mConfig->deleteGroup( "Resource_" + resource->identifier() );
  mConfig->sync();
}

Resource* ManagerImpl::getResource( const QString& identifier )
{
  Resource::List::ConstIterator it;
  for ( it = mResources.begin(); it != mResources.end(); ++it ) {
    if ( (*it)->identifier() == identifier )
      return *it;
  }
  return 0;
}

QString ManagerImpl::defaultConfigFile( const QString &family )
{
  return locateLocal( "config",
                      QString( "kresources/%1/stdrc" ).arg( family ) );
}

#include "managerimpl.moc"
