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
#include <klocalizedstring.h>
#include <qdebug.h>
#include <kconfig.h>
#include <kuser.h>
#include <kconfiggroup.h>

#include <QList>
#include <QRegExp>
#include <QtDBus/QtDBus>
#include <QHostInfo>

#include <assert.h>
#include <krandom.h>

#include "identitymanageradaptor.h"

namespace KPIMIdentities
{

static QString newDBusObjectName()
{
    static int s_count = 0;
    QString name(QStringLiteral("/KPIMIDENTITIES_IdentityManager"));
    if (s_count++) {
        name += QLatin1Char('_');
        name += QString::number(s_count);
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
    Private(KPIMIdentities::IdentityManager *);
    void writeConfig() const;
    void readConfig(KConfig *config);
    void createDefaultIdentity();
    QStringList groupList(KConfig *config) const;
    void slotIdentitiesChanged(const QString &id);
    KConfig *mConfig;

    QList<Identity> mIdentities;
    QList<Identity> shadowIdentities;

    // returns a new Unique Object Identifier
    int newUoid();

    bool mReadOnly;
    KPIMIdentities::IdentityManager *q;
};

IdentityManager::Private::Private(KPIMIdentities::IdentityManager *manager)
    : q(manager)
{
}

void IdentityManager::Private::writeConfig() const
{
    const QStringList identities = groupList(mConfig);
    QStringList::const_iterator groupEnd = identities.constEnd();
    for (QStringList::const_iterator group = identities.constBegin();
            group != groupEnd; ++group) {
        mConfig->deleteGroup(*group);
    }
    int i = 0;
    ConstIterator end = mIdentities.constEnd();
    for (ConstIterator it = mIdentities.constBegin();
            it != end; ++it, ++i) {
        KConfigGroup cg(mConfig, QString::fromLatin1("Identity #%1").arg(i));
        (*it).writeConfig(cg);
        if ((*it).isDefault()) {
            // remember which one is default:
            KConfigGroup general(mConfig, "General");
            general.writeEntry(configKeyDefaultIdentity, (*it).uoid());

            // Also write the default identity to emailsettings
            KEMailSettings es;
            es.setSetting(KEMailSettings::RealName, (*it).fullName());
            es.setSetting(KEMailSettings::EmailAddress, (*it).primaryEmailAddress());
            es.setSetting(KEMailSettings::Organization, (*it).organization());
            es.setSetting(KEMailSettings::ReplyToAddress, (*it).replyToAddr());
        }
    }
    mConfig->sync();

}

void IdentityManager::Private::readConfig(KConfig *config)
{
    mIdentities.clear();

    const QStringList identities = groupList(config);
    if (identities.isEmpty()) {
        return; // nothing to be done...
    }

    KConfigGroup general(config, "General");
    uint defaultIdentity = general.readEntry(configKeyDefaultIdentity, 0);
    bool haveDefault = false;
    QStringList::const_iterator groupEnd = identities.constEnd();
    for (QStringList::const_iterator group = identities.constBegin();
            group != groupEnd; ++group) {
        KConfigGroup configGroup(config, *group);
        mIdentities << Identity();
        mIdentities.last().readConfig(configGroup);
        if (!haveDefault && mIdentities.last().uoid() == defaultIdentity) {
            haveDefault = true;
            mIdentities.last().setIsDefault(true);
        }
    }

    if (!haveDefault) {
        qWarning() << "IdentityManager: There was no default identity."
                   << "Marking first one as default.";
        mIdentities.first().setIsDefault(true);
    }
    qSort(mIdentities);

    shadowIdentities = mIdentities;
}

void IdentityManager::Private::createDefaultIdentity()
{
    QString fullName, emailAddress;
    bool done = false;

    // Check if the application has any settings
    q->createDefaultIdentity(fullName, emailAddress);

    // If not, then use the kcontrol settings
    if (fullName.isEmpty() && emailAddress.isEmpty()) {
        KEMailSettings emailSettings;
        fullName = emailSettings.getSetting(KEMailSettings::RealName);
        emailAddress = emailSettings.getSetting(KEMailSettings::EmailAddress);

        if (!fullName.isEmpty() && !emailAddress.isEmpty()) {
            q->newFromControlCenter(i18nc("use default address from control center",
                                          "Default"));
            done = true;
        } else {
            // If KEmailSettings doesn't have name and address, generate something from KUser
            KUser user;
            if (fullName.isEmpty()) {
                fullName = user.property(KUser::FullName).toString();
            }
            if (emailAddress.isEmpty()) {
                emailAddress = user.loginName();
                if (!emailAddress.isEmpty()) {
                    KConfigGroup general(mConfig, "General");
                    QString defaultdomain = general.readEntry("Default domain");
                    if (!defaultdomain.isEmpty()) {
                        emailAddress += QLatin1Char('@') + defaultdomain;
                    } else {
                        emailAddress.clear();
                    }
                }
            }
        }
    }

    if (!done) {
        // Default identity name
        QString name(i18nc("Default name for new email accounts/identities.", "Unnamed"));

        if (!emailAddress.isEmpty()) {
            // If we have an email address, create a default identity name from it
            QString idName = emailAddress;
            int pos = idName.indexOf(QLatin1Char('@'));
            if (pos != -1) {
                name = idName.mid(pos + 1, -1);
            }

            // Make the name a bit more human friendly
            name.replace(QLatin1Char('.'), QLatin1Char(' '));
            pos = name.indexOf(QLatin1Char(' '));
            if (pos != 0) {
                name[pos + 1] = name[pos + 1].toUpper();
            }
            name[0] = name[0].toUpper();
        } else if (!fullName.isEmpty()) {
            // If we have a full name, create a default identity name from it
            name = fullName;
        }
        shadowIdentities << Identity(name, fullName, emailAddress);
    }

    shadowIdentities.last().setIsDefault(true);
    shadowIdentities.last().setUoid(newUoid());
    if (mReadOnly) {   // commit won't do it in readonly mode
        mIdentities = shadowIdentities;
    }
}

QStringList IdentityManager::Private::groupList(KConfig *config) const
{
    return config->groupList().filter(QRegExp(QStringLiteral("^Identity #\\d+$")));
}

int IdentityManager::Private::newUoid()
{
    int uoid;

    // determine the UOIDs of all saved identities
    QList<uint> usedUOIDs;
    QList<Identity>::ConstIterator end(mIdentities.constEnd());
    for (QList<Identity>::ConstIterator it = mIdentities.constBegin();
            it != end; ++it) {
        usedUOIDs << (*it).uoid();
    }

    if (q->hasPendingChanges()) {
        // add UOIDs of all shadow identities. Yes, we will add a lot of duplicate
        // UOIDs, but avoiding duplicate UOIDs isn't worth the effort.
        QList<Identity>::ConstIterator endShadow(shadowIdentities.constEnd());
        for (QList<Identity>::ConstIterator it = shadowIdentities.constBegin();
                it != endShadow; ++it) {
            usedUOIDs << (*it).uoid();
        }
    }

    usedUOIDs << 0; // no UOID must be 0 because this value always refers to the
    // default identity

    do {
        uoid = KRandom::random();
    } while (usedUOIDs.indexOf(uoid) != -1);

    return uoid;
}

void IdentityManager::Private::slotIdentitiesChanged(const QString &id)
{
    qDebug() << " KPIMIdentities::IdentityManager::slotIdentitiesChanged :" << id;
    const QString ourIdentifier = QString::fromLatin1("%1/%2").
                                  arg(QDBusConnection::sessionBus().baseService()).
                                  arg(q->property("uniqueDBusPath").toString());
    if (id != ourIdentifier) {
        mConfig->reparseConfiguration();
        Q_ASSERT(!q->hasPendingChanges());
        readConfig(mConfig);
        emit q->changed();
    }
}

IdentityManager::IdentityManager(bool readonly, QObject *parent,
                                 const char *name)
    : QObject(parent),
      d(new Private(this))
{
    setObjectName(QLatin1String(name));
    new IdentityManagerAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    const QString dbusPath = newDBusObjectName();
    setProperty("uniqueDBusPath", dbusPath);
    const QString dbusInterface = QStringLiteral("org.kde.pim.IdentityManager");
    dbus.registerObject(dbusPath, this);
    dbus.connect(QString(), QString(), dbusInterface, QStringLiteral("identitiesChanged"), this,
                 SLOT(slotIdentitiesChanged(QString)));

    d->mReadOnly = readonly;
    d->mConfig = new KConfig(QStringLiteral("emailidentities"));
    d->readConfig(d->mConfig);
    if (d->mIdentities.isEmpty()) {
        qDebug() << "emailidentities is empty -> convert from kmailrc";
        // No emailidentities file, or an empty one due to broken conversion
        // (kconf_update bug in kdelibs <= 3.2.2)
        // => convert it, i.e. read settings from kmailrc
        KConfig kmailConf(QStringLiteral("kmailrc"));
        d->readConfig(&kmailConf);
    }
    // we need at least a default identity:
    if (d->mIdentities.isEmpty()) {
        qDebug() << "IdentityManager: No identity found. Creating default.";
        d->createDefaultIdentity();
        commit();
    }

    KConfig kmailConf(QStringLiteral("kmail2rc"));
    if (!d->mReadOnly) {
        bool needCommit = false;
        if (kmailConf.hasGroup(QStringLiteral("Composer"))) {
            KConfigGroup composerGroup = kmailConf.group(QStringLiteral("Composer"));
            if (composerGroup.hasKey(QStringLiteral("pgp-auto-sign"))) {
                const bool pgpAutoSign = composerGroup.readEntry(QStringLiteral("pgp-auto-sign"), false);
                QList<Identity>::iterator end = d->mIdentities.end();
                for (QList<Identity>::iterator it = d->mIdentities.begin(); it != end; ++it) {
                    it->setPgpAutoSign(pgpAutoSign);
                }
                composerGroup.deleteEntry(QStringLiteral("pgp-auto-sign"));
                composerGroup.sync();
                needCommit = true;
            }
        }
        if (kmailConf.hasGroup(QStringLiteral("General"))) {
            KConfigGroup generalGroup = kmailConf.group(QStringLiteral("General"));
            if (generalGroup.hasKey(QStringLiteral("Default domain"))) {
                QString defaultDomain = generalGroup.readEntry(QStringLiteral("Default domain"));
                if (defaultDomain.isEmpty()) {
                    defaultDomain = QHostInfo::localHostName();
                }
                QList<Identity>::iterator end = d->mIdentities.end();
                for (QList<Identity>::iterator it = d->mIdentities.begin(); it != end; ++it) {
                    it->setDefaultDomainName(defaultDomain);
                }
                generalGroup.deleteEntry(QStringLiteral("Default domain"));
                generalGroup.sync();
                needCommit = true;
            }
        }
        if (needCommit) {
            commit();
        }
    }

    // Migration: people without settings in kemailsettings should get some
    if (KEMailSettings().getSetting(KEMailSettings::EmailAddress).isEmpty()) {
        d->writeConfig();
    }
}

IdentityManager::~IdentityManager()
{
    if (hasPendingChanges()) {
        qWarning() << "IdentityManager: There were uncommitted changes!";
    }
    delete d;
}

QString IdentityManager::makeUnique(const QString &name) const
{
    int suffix = 1;
    QString result = name;
    while (identities().contains(result)) {
        result = i18nc("%1: name; %2: number appended to it to make it unique "
                       "among a list of names", "%1 #%2",
                       name, suffix);
        ++suffix;
    }
    return result;
}

bool IdentityManager::isUnique(const QString &name) const
{
    return !identities().contains(name);
}

void IdentityManager::commit()
{
    // early out:
    if (!hasPendingChanges() || d->mReadOnly) {
        return;
    }

    QList<uint> seenUOIDs;
    QList<Identity>::ConstIterator end = d->mIdentities.constEnd();
    for (QList<Identity>::ConstIterator it = d->mIdentities.constBegin();
            it != end; ++it) {
        seenUOIDs << (*it).uoid();
    }

    QList<uint> changedUOIDs;
    // find added and changed identities:
    for (QList<Identity>::ConstIterator it = d->shadowIdentities.constBegin();
            it != d->shadowIdentities.constEnd(); ++it) {
        int index = seenUOIDs.indexOf((*it).uoid());
        if (index != -1) {
            uint uoid = seenUOIDs.at(index);
            const Identity &orig = identityForUoid(uoid);    // look up in mIdentities
            if (*it != orig) {
                // changed identity
                qDebug() << "emitting changed() for identity" << uoid;
                emit changed(*it);
                changedUOIDs << uoid;
            }
            seenUOIDs.removeAll(uoid);
        } else {
            // new identity
            qDebug() << "emitting added() for identity" << (*it).uoid();
            emit added(*it);
        }
    }

    // what's left are deleted identities:
    for (QList<uint>::ConstIterator it = seenUOIDs.constBegin();
            it != seenUOIDs.constEnd(); ++it) {
        qDebug() << "emitting deleted() for identity" << (*it);
        emit deleted(*it);
    }

    d->mIdentities = d->shadowIdentities;
    d->writeConfig();

    // now that mIdentities has all the new info, we can emit the added/changed
    // signals that ship a uoid. This is because the slots might use
    // identityForUoid(uoid)...
    QList<uint>::ConstIterator changedEnd(changedUOIDs.constEnd());
    for (QList<uint>::ConstIterator it = changedUOIDs.constBegin();
            it != changedEnd; ++it) {
        emit changed(*it);
    }

    emit changed(); // normal signal

    // DBus signal for other IdentityManager instances
    const QString ourIdentifier = QString::fromLatin1("%1/%2").
                                  arg(QDBusConnection::sessionBus().baseService()).
                                  arg(property("uniqueDBusPath").toString());
    emit identitiesChanged(ourIdentifier);
}

void IdentityManager::rollback()
{
    d->shadowIdentities = d->mIdentities;
}

bool IdentityManager::hasPendingChanges() const
{
    return d->mIdentities != d->shadowIdentities;
}

QStringList IdentityManager::identities() const
{
    QStringList result;
    ConstIterator end = d->mIdentities.constEnd();
    for (ConstIterator it = d->mIdentities.constBegin();
            it != end; ++it) {
        result << (*it).identityName();
    }
    return result;
}

QStringList IdentityManager::shadowIdentities() const
{
    QStringList result;
    ConstIterator end = d->shadowIdentities.constEnd();
    for (ConstIterator it = d->shadowIdentities.constBegin();
            it != end; ++it) {
        result << (*it).identityName();
    }
    return result;
}

void IdentityManager::sort()
{
    qSort(d->shadowIdentities);
}

IdentityManager::ConstIterator IdentityManager::begin() const
{
    return d->mIdentities.constBegin();
}

IdentityManager::ConstIterator IdentityManager::end() const
{
    return d->mIdentities.constEnd();
}

IdentityManager::Iterator IdentityManager::modifyBegin()
{
    return d->shadowIdentities.begin();
}

IdentityManager::Iterator IdentityManager::modifyEnd()
{
    return d->shadowIdentities.end();
}

const Identity &IdentityManager::identityForUoid(uint uoid) const
{
    for (ConstIterator it = begin(); it != end(); ++it) {
        if ((*it).uoid() == uoid) {
            return (*it);
        }
    }
    return Identity::null();
}

const Identity &IdentityManager::identityForUoidOrDefault(uint uoid) const
{
    const Identity &ident = identityForUoid(uoid);
    if (ident.isNull()) {
        return defaultIdentity();
    } else {
        return ident;
    }
}

const Identity &IdentityManager::identityForAddress(
    const QString &addresses) const
{
    const QStringList addressList = KPIMUtils::splitAddressList(addresses);
    foreach (const QString &fullAddress, addressList) {
        const QString addrSpec = KPIMUtils::extractEmailAddress(fullAddress).toLower();
        for (ConstIterator it = begin(); it != end(); ++it) {
            const Identity &identity = *it;
            if (identity.matchesEmailAddress(addrSpec)) {
                return identity;
            }
        }
    }
    return Identity::null();
}

bool IdentityManager::thatIsMe(const QString &addressList) const
{
    return !identityForAddress(addressList).isNull();
}

Identity &IdentityManager::modifyIdentityForName(const QString &name)
{
    for (Iterator it = modifyBegin(); it != modifyEnd(); ++it) {
        if ((*it).identityName() == name) {
            return (*it);
        }
    }

    qWarning() << "IdentityManager::modifyIdentityForName() used as"
               << "newFromScratch() replacement!"
               << endl << "  name == \"" << name << "\"";
    return newFromScratch(name);
}

Identity &IdentityManager::modifyIdentityForUoid(uint uoid)
{
    for (Iterator it = modifyBegin(); it != modifyEnd(); ++it) {
        if ((*it).uoid() == uoid) {
            return (*it);
        }
    }

    qWarning() << "IdentityManager::identityForUoid() used as"
               << "newFromScratch() replacement!"
               << endl << "  uoid == \"" << uoid << "\"";
    return newFromScratch(i18n("Unnamed"));
}

const Identity &IdentityManager::defaultIdentity() const
{
    for (ConstIterator it = begin(); it != end(); ++it) {
        if ((*it).isDefault()) {
            return (*it);
        }
    }

    if (d->mIdentities.isEmpty()) {
        qCritical() << "IdentityManager: No default identity found!";
    } else {
        qWarning() << "IdentityManager: No default identity found!";
    }
    return *begin();
}

bool IdentityManager::setAsDefault(uint uoid)
{
    // First, check if the identity actually exists:
    bool found = false;
    for (ConstIterator it = d->shadowIdentities.constBegin();
            it != d->shadowIdentities.constEnd(); ++it) {
        if ((*it).uoid() == uoid) {
            found = true;
            break;
        }
    }

    if (!found) {
        return false;
    }

    // Then, change the default as requested:
    for (Iterator it = modifyBegin(); it != modifyEnd(); ++it) {
        (*it).setIsDefault((*it).uoid() == uoid);
    }

    // and re-sort:
    sort();
    return true;
}

bool IdentityManager::removeIdentity(const QString &name)
{
    if (d->shadowIdentities.size() <= 1) {
        return false;
    }

    for (Iterator it = modifyBegin(); it != modifyEnd(); ++it) {
        if ((*it).identityName() == name) {
            bool removedWasDefault = (*it).isDefault();
            d->shadowIdentities.erase(it);
            if (removedWasDefault && !d->shadowIdentities.isEmpty()) {
                d->shadowIdentities.first().setIsDefault(true);
            }
            return true;
        }
    }
    return false;
}

bool IdentityManager::removeIdentityForced(const QString &name)
{
    for (Iterator it = modifyBegin(); it != modifyEnd(); ++it) {
        if ((*it).identityName() == name) {
            bool removedWasDefault = (*it).isDefault();
            d->shadowIdentities.erase(it);
            if (removedWasDefault && !d->shadowIdentities.isEmpty()) {
                d->shadowIdentities.first().setIsDefault(true);
            }
            return true;
        }
    }
    return false;
}

Identity &IdentityManager::newFromScratch(const QString &name)
{
    return newFromExisting(Identity(name));
}

Identity &IdentityManager::newFromControlCenter(const QString &name)
{
    KEMailSettings es;
    es.setProfile(es.defaultProfileName());

    return
        newFromExisting(Identity(name,
                                 es.getSetting(KEMailSettings::RealName),
                                 es.getSetting(KEMailSettings::EmailAddress),
                                 es.getSetting(KEMailSettings::Organization),
                                 es.getSetting(KEMailSettings::ReplyToAddress)));
}

Identity &IdentityManager::newFromExisting(const Identity &other, const QString &name)
{
    d->shadowIdentities << other;
    Identity &result = d->shadowIdentities.last();
    result.setIsDefault(false);    // we don't want two default identities!
    result.setUoid(d->newUoid());    // we don't want two identies w/ same UOID
    if (!name.isNull()) {
        result.setIdentityName(name);
    }
    return result;
}

QStringList KPIMIdentities::IdentityManager::allEmails() const
{
    QStringList lst;
    for (ConstIterator it = begin(); it != end(); ++it) {
        lst << (*it).primaryEmailAddress();
        if (!(*it).emailAliases().isEmpty()) {
            lst << (*it).emailAliases();
        }
    }
    return lst;
}

void KPIMIdentities::IdentityManager::slotRollback()
{
    rollback();
}

}
#include "moc_identitymanager.cpp"
