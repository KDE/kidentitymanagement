/*
    SPDX-FileCopyrightText: 2002 Marc Mutz <mutz@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

// config keys:
static const char configKeyDefaultIdentity[] = "Default Identity";

#include "identitymanager.h"
using namespace Qt::Literals::StringLiterals;

#include "identity.h" // for IdentityList::{export,import}Data

#include <KEmailAddress> // for static helper functions

#include "kidentitymanagementcore_debug.h"
#include <KConfig>
#include <KConfigGroup>
#include <KEMailSettings> // for IdentityEntry::fromControlCenter()
#include <KLocalizedString>
#include <KSharedConfig>
#include <kuser.h>

#include <QDBusConnection>
#include <QHostInfo>
#include <QRandomGenerator>

#include <QRegularExpression>
#include <cassert>

#include "identitymanageradaptor.h"

namespace KIdentityManagementCore
{
static QString newDBusObjectName()
{
    static int s_count = 0;
    QString name(u"/KIDENTITYMANAGER_IdentityManager"_s);
    if (s_count++) {
        name += u'_';
        name += QString::number(s_count);
    }
    return name;
}

/**
 *   Private class that helps to provide binary compatibility between releases.
 *   @internal
 */
//@cond PRIVATE
class IdentityManagerPrivate
{
public:
    explicit IdentityManagerPrivate(KIdentityManagementCore::IdentityManager *);
    ~IdentityManagerPrivate();
    void writeConfig() const;
    void readConfig(KConfig *config);
    void createDefaultIdentity();
    [[nodiscard]] QStringList groupList(KConfig *config) const;
    void slotIdentitiesChanged(const QString &id);
    KConfig *mConfig = nullptr;

    QList<Identity> mIdentities;
    QList<Identity> shadowIdentities;

    // returns a new Unique Object Identifier
    [[nodiscard]] int newUoid();

    bool mReadOnly = true;
    KIdentityManagementCore::IdentityManager *const q;
};

IdentityManagerPrivate::IdentityManagerPrivate(KIdentityManagementCore::IdentityManager *manager)
    : q(manager)
{
}

void IdentityManagerPrivate::writeConfig() const
{
    const QStringList identities = groupList(mConfig);
    for (const auto &group : identities) {
        mConfig->deleteGroup(group);
    }
    int i = 0;
    IdentityManager::ConstIterator end = mIdentities.constEnd();
    for (IdentityManager::ConstIterator it = mIdentities.constBegin(); it != end; ++it, ++i) {
        KConfigGroup cg(mConfig, u"Identity #%1"_s.arg(i));
        (*it).writeConfig(cg);
        if ((*it).isDefault()) {
            // remember which one is default:
            KConfigGroup general(mConfig, u"General"_s);
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

void IdentityManagerPrivate::readConfig(KConfig *config)
{
    mIdentities.clear();

    const QStringList identities = groupList(config);
    if (identities.isEmpty()) {
        return; // nothing to be done...
    }

    KConfigGroup general(config, u"General"_s);
    uint defaultIdentity = general.readEntry(configKeyDefaultIdentity, 0);
    bool haveDefault = false;
    for (const QString &group : identities) {
        KConfigGroup configGroup(config, group);
        Identity identity;
        identity.readConfig(configGroup);
        // Don't load invalid identity
        if (!identity.isNull() && !identity.primaryEmailAddress().isEmpty()) {
            if (!haveDefault && identity.uoid() == defaultIdentity) {
                haveDefault = true;
                identity.setIsDefault(true);
            }
        }
        mIdentities << identity;
    }
    if (!haveDefault) {
        if (mIdentities.isEmpty()) {
            mIdentities << Identity();
        }

        qCDebug(KIDENTITYMANAGEMENT_LOG) << "IdentityManager: There was no default identity."
                                         << "Marking first one as default.";
        mIdentities.first().setIsDefault(true);
    }
    std::sort(mIdentities.begin(), mIdentities.end());

    shadowIdentities = mIdentities;
}

void IdentityManagerPrivate::createDefaultIdentity()
{
    QString fullName;
    QString emailAddress;
    bool done = false;

    // Check if the application has any settings
    q->createDefaultIdentity(fullName, emailAddress);

    // If not, then use the kcontrol settings
    if (fullName.isEmpty() && emailAddress.isEmpty()) {
        KEMailSettings emailSettings;
        fullName = emailSettings.getSetting(KEMailSettings::RealName);
        emailAddress = emailSettings.getSetting(KEMailSettings::EmailAddress);

        if (!fullName.isEmpty() && !emailAddress.isEmpty()) {
            q->newFromControlCenter(i18nc("use default address from control center", "Default"));
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
                    KConfigGroup general(mConfig, u"General"_s);
                    QString defaultdomain = general.readEntry("Default domain");
                    if (!defaultdomain.isEmpty()) {
                        emailAddress += u'@' + defaultdomain;
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
            int pos = idName.indexOf(u'@');
            if (pos != -1) {
                name = idName.mid(pos + 1, -1);
            }

            // Make the name a bit more human friendly
            name.replace(u'.', u' ');
            pos = name.indexOf(u' ');
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
    if (mReadOnly) { // commit won't do it in readonly mode
        mIdentities = shadowIdentities;
    }
}

QStringList IdentityManagerPrivate::groupList(KConfig *config) const
{
    static const QRegularExpression regExpr(u"^Identity #\\d+$"_s);
    return config->groupList().filter(regExpr);
}

int IdentityManagerPrivate::newUoid()
{
    int uoid;

    // determine the UOIDs of all saved identities
    QList<uint> usedUOIDs;
    usedUOIDs.reserve(mIdentities.count() + (q->hasPendingChanges() ? shadowIdentities.count() : 0));
    const QList<Identity>::ConstIterator end(mIdentities.constEnd());
    for (QList<Identity>::ConstIterator it = mIdentities.constBegin(); it != end; ++it) {
        usedUOIDs << (*it).uoid();
    }

    if (q->hasPendingChanges()) {
        // add UOIDs of all shadow identities. Yes, we will add a lot of duplicate
        // UOIDs, but avoiding duplicate UOIDs isn't worth the effort.
        const QList<Identity>::ConstIterator endShadow(shadowIdentities.constEnd());
        for (QList<Identity>::ConstIterator it = shadowIdentities.constBegin(); it != endShadow; ++it) {
            usedUOIDs << (*it).uoid();
        }
    }

    do {
        // 0 refers to the default identity, so accept 1 only as lowest value
        uoid = QRandomGenerator::global()->bounded(1, RAND_MAX);
    } while (usedUOIDs.contains(uoid));

    return uoid;
}

void IdentityManagerPrivate::slotIdentitiesChanged(const QString &id)
{
    qCDebug(KIDENTITYMANAGEMENT_LOG) << " KIdentityManagementCore::IdentityManager::slotIdentitiesChanged :" << id;
    const QString ourIdentifier = u"%1/%2"_s.arg(QDBusConnection::sessionBus().baseService(), q->property("uniqueDBusPath").toString());
    if (id != ourIdentifier) {
        mConfig->reparseConfiguration();
        Q_ASSERT(!q->hasPendingChanges());
        readConfig(mConfig);
        Q_EMIT q->needToReloadIdentitySettings();
        Q_EMIT q->changed();
        Q_EMIT q->identitiesWereChanged();
    }
}

Q_GLOBAL_STATIC(IdentityManager, s_self)

IdentityManager *IdentityManager::self()
{
    return s_self;
}

IdentityManager::IdentityManager(bool readonly, QObject *parent, const char *name)
    : QObject(parent)
    , d(new IdentityManagerPrivate(this))
{
    setObjectName(QLatin1StringView(name));
    new IdentityManagerAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    const QString dbusPath = newDBusObjectName();
    setProperty("uniqueDBusPath", dbusPath);
    const QString dbusInterface = u"org.kde.pim.IdentityManager"_s;
    dbus.registerObject(dbusPath, this);
    dbus.connect(QString(), QString(), dbusInterface, u"identitiesChanged"_s, this, SLOT(slotIdentitiesChanged(QString)));

    d->mReadOnly = readonly;
    d->mConfig = new KConfig(u"emailidentities"_s);
    if (!d->mConfig->isConfigWritable(true)) {
        qCWarning(KIDENTITYMANAGEMENT_LOG) << "impossible to write on this file";
    }
    d->readConfig(d->mConfig);
    // we need at least a default identity:
    if (d->mIdentities.isEmpty()) {
        qCDebug(KIDENTITYMANAGEMENT_LOG) << "IdentityManager: No identity found. Creating default.";
        d->createDefaultIdentity();
        commit();
    }

    KSharedConfig::Ptr kmailConf(KSharedConfig::openConfig(u"kmail2rc"_s));
    if (!d->mReadOnly) {
        bool needCommit = false;
        if (kmailConf->hasGroup(u"Composer"_s)) {
            KConfigGroup composerGroup = kmailConf->group(u"Composer"_s);
            if (composerGroup.hasKey(u"pgp-auto-sign"_s)) {
                const bool pgpAutoSign = composerGroup.readEntry(u"pgp-auto-sign"_s, false);
                const QList<Identity>::iterator end = d->mIdentities.end();
                for (QList<Identity>::iterator it = d->mIdentities.begin(); it != end; ++it) {
                    it->setPgpAutoSign(pgpAutoSign);
                }
                composerGroup.deleteEntry(u"pgp-auto-sign"_s);
                composerGroup.sync();
                needCommit = true;
            }
        }
        if (kmailConf->hasGroup(u"General"_s)) {
            KConfigGroup generalGroup = kmailConf->group(u"General"_s);
            if (generalGroup.hasKey(u"Default domain"_s)) {
                QString defaultDomain = generalGroup.readEntry(u"Default domain"_s);
                if (defaultDomain.isEmpty()) {
                    defaultDomain = QHostInfo::localHostName();
                }
                const QList<Identity>::iterator end = d->mIdentities.end();
                for (QList<Identity>::iterator it = d->mIdentities.begin(); it != end; ++it) {
                    it->setDefaultDomainName(defaultDomain);
                }
                generalGroup.deleteEntry(u"Default domain"_s);
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
        qCWarning(KIDENTITYMANAGEMENT_LOG) << "IdentityManager: There were uncommitted changes!";
    }
}

QString IdentityManager::makeUnique(const QString &name) const
{
    int suffix = 1;
    QString result = name;
    while (identities().contains(result)) {
        result = i18nc(
            "%1: name; %2: number appended to it to make it unique "
            "among a list of names",
            "%1 #%2",
            name,
            suffix);
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
    seenUOIDs.reserve(d->mIdentities.count());
    const QList<Identity>::ConstIterator end = d->mIdentities.constEnd();
    for (QList<Identity>::ConstIterator it = d->mIdentities.constBegin(); it != end; ++it) {
        seenUOIDs << (*it).uoid();
    }

    QList<uint> changedUOIDs;
    // find added and changed identities:
    for (QList<Identity>::ConstIterator it = d->shadowIdentities.constBegin(); it != d->shadowIdentities.constEnd(); ++it) {
        const int index = seenUOIDs.indexOf((*it).uoid());
        if (index != -1) {
            uint uoid = seenUOIDs.at(index);
            const Identity &orig = identityForUoid(uoid); // look up in mIdentities
            if (*it != orig) {
                // changed identity
                qCDebug(KIDENTITYMANAGEMENT_LOG) << "emitting changed() for identity" << uoid;
                Q_EMIT changed(*it);
                Q_EMIT identityChanged(*it);
                changedUOIDs << uoid;
            }
            seenUOIDs.removeAll(uoid);
        } else {
            // new identity
            qCDebug(KIDENTITYMANAGEMENT_LOG) << "emitting added() for identity" << (*it).uoid();
            Q_EMIT added(*it);
        }
    }

    d->mIdentities = d->shadowIdentities;

    // we cannot throw the signal earlier as it would lead to a call to
    // modifyIdentityForUoid, in turn creating a new entry for the one we are
    // trying to destroy...
    for (QList<uint>::ConstIterator it = seenUOIDs.constBegin(); it != seenUOIDs.constEnd(); ++it) {
        qCDebug(KIDENTITYMANAGEMENT_LOG) << "emitting deleted() for identity" << (*it);
        Q_EMIT deleted(*it);
    }

    d->writeConfig();

    // now that mIdentities has all the new info, we can Q_EMIT the added/changed
    // signals that ship a uoid. This is because the slots might use
    // identityForUoid(uoid)...
    QList<uint>::ConstIterator changedEnd(changedUOIDs.constEnd());
    for (QList<uint>::ConstIterator it = changedUOIDs.constBegin(); it != changedEnd; ++it) {
        Q_EMIT changed(*it);
    }

    Q_EMIT changed(); // normal signal
    Q_EMIT identitiesWereChanged(); // normal signal

    // DBus signal for other IdentityManager instances
    const QString ourIdentifier = u"%1/%2"_s.arg(QDBusConnection::sessionBus().baseService(), property("uniqueDBusPath").toString());
    Q_EMIT identitiesChanged(ourIdentifier);
}

void IdentityManager::rollback()
{
    d->shadowIdentities = d->mIdentities;
}

void IdentityManager::saveIdentity(const Identity &ident)
{
    const auto existing = std::find_if(modifyBegin(), modifyEnd(), [ident](const auto &existingIdentity) {
        return existingIdentity.uoid() == ident.uoid();
    });

    if (existing != modifyEnd()) {
        d->shadowIdentities.replace(existing - modifyBegin(), ident);
    } else {
        d->shadowIdentities << ident;
    }

    commit();
}

bool IdentityManager::hasPendingChanges() const
{
    return d->mIdentities != d->shadowIdentities;
}

QStringList IdentityManager::identities() const
{
    QStringList result;
    result.reserve(d->mIdentities.count());
    ConstIterator end = d->mIdentities.constEnd();
    for (ConstIterator it = d->mIdentities.constBegin(); it != end; ++it) {
        result << (*it).identityName();
    }
    return result;
}

QStringList IdentityManager::shadowIdentities() const
{
    QStringList result;
    result.reserve(d->shadowIdentities.count());
    ConstIterator end = d->shadowIdentities.constEnd();
    for (ConstIterator it = d->shadowIdentities.constBegin(); it != end; ++it) {
        result << (*it).identityName();
    }
    return result;
}

void IdentityManager::sort()
{
    std::sort(d->shadowIdentities.begin(), d->shadowIdentities.end());
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
            return *it;
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

const Identity &IdentityManager::identityForAddress(const QString &addresses) const
{
    const QStringList addressList = KEmailAddress::splitAddressList(addresses);
    for (const QString &fullAddress : addressList) {
        const QString addrSpec = KEmailAddress::extractEmailAddress(fullAddress).toLower();
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
            return *it;
        }
    }

    qCWarning(KIDENTITYMANAGEMENT_LOG) << "IdentityManager::modifyIdentityForName() used as"
                                       << "newFromScratch() replacement!" << Qt::endl
                                       << "  name == \"" << name << "\"";
    return newFromScratch(name);
}

Identity &IdentityManager::modifyIdentityForUoid(uint uoid)
{
    for (Iterator it = modifyBegin(); it != modifyEnd(); ++it) {
        if ((*it).uoid() == uoid) {
            return *it;
        }
    }

    qCWarning(KIDENTITYMANAGEMENT_LOG) << "IdentityManager::identityForUoid() used as"
                                       << "newFromScratch() replacement!" << Qt::endl
                                       << "  uoid == \"" << uoid << "\"";
    return newFromScratch(i18n("Unnamed"));
}

const Identity &IdentityManager::defaultIdentity() const
{
    for (ConstIterator it = begin(); it != end(); ++it) {
        if ((*it).isDefault()) {
            return *it;
        }
    }

    if (d->mIdentities.isEmpty()) {
        qCritical() << "IdentityManager: No default identity found!";
    } else {
        qCWarning(KIDENTITYMANAGEMENT_LOG) << "IdentityManager: No default identity found!";
    }
    return *begin();
}

bool IdentityManager::setAsDefault(uint uoid)
{
    // First, check if the identity actually exists:
    bool found = false;
    ConstIterator end(d->shadowIdentities.constEnd());
    for (ConstIterator it = d->shadowIdentities.constBegin(); it != end; ++it) {
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

    return newFromExisting(Identity(name,
                                    es.getSetting(KEMailSettings::RealName),
                                    es.getSetting(KEMailSettings::EmailAddress),
                                    es.getSetting(KEMailSettings::Organization),
                                    es.getSetting(KEMailSettings::ReplyToAddress)));
}

Identity &IdentityManager::newFromExisting(const Identity &other, const QString &name)
{
    d->shadowIdentities << other;
    Identity &result = d->shadowIdentities.last();
    result.setIsDefault(false); // we don't want two default identities!
    result.setUoid(d->newUoid()); // we don't want two identities w/ same UOID
    if (!name.isNull()) {
        result.setIdentityName(name);
    }
    return result;
}

QStringList KIdentityManagementCore::IdentityManager::allEmails() const
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

void IdentityManager::createDefaultIdentity(QString &, QString &)
{
}

void KIdentityManagementCore::IdentityManager::slotRollback()
{
    rollback();
}

IdentityManagerPrivate::~IdentityManagerPrivate()
{
    delete mConfig;
}
}
#include "moc_identitymanager.cpp"
