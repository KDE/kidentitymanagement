/*
  SPDX-FileCopyrightText: 2014 Sandro Knauß <knauss@kolabsys.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "utils.h"
#include "identitymanager.h"

#include <QObject>

using namespace KIdentityManagement;

class IdendentitiesCache : public QObject
{
    Q_OBJECT
public:
    explicit IdendentitiesCache(QObject *parent = nullptr)
        : QObject(parent)
        , mIdentityManager(/*ro=*/true)
    {
        connect(&mIdentityManager, qOverload<>(&IdentityManager::changed), this, &IdendentitiesCache::slotIdentitiesChanged);
        slotIdentitiesChanged();
    }

    Q_REQUIRED_RESULT const QSet<QString> &emails() const
    {
        return mEmails;
    }

private:
    void slotIdentitiesChanged()
    {
        mEmails.clear();
        const QStringList lst = mIdentityManager.allEmails();
        mEmails.reserve(lst.count());
        for (const QString &email : lst) {
            mEmails.insert(email.toLower());
        }
    }

    IdentityManager mIdentityManager;
    QSet<QString> mEmails;
};

Q_GLOBAL_STATIC(IdendentitiesCache, sIdentitiesCache)

bool KIdentityManagement::thatIsMe(const QString &email)
{
    return allEmails().contains(email.toLower());
}

const QSet<QString> &KIdentityManagement::allEmails()
{
    return sIdentitiesCache()->emails();
}

#include "utils.moc"