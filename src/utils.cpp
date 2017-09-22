/*
  Copyright (c) 2014 Sandro Knauß <knauss@kolabsys.com>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or ( at your
  option ) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
*/

#include "utils.h"
#include "identitymanager.h"

#include <QObject>

using namespace KIdentityManagement;

class IdendentitiesCache : public QObject
{
    Q_OBJECT
public:
    IdendentitiesCache()
        : mIdentityManager(/*ro=*/ true)
    {
        connect(&mIdentityManager, QOverload<>::of(&IdentityManager::changed), this, &IdendentitiesCache::slotIdentitiesChanged);
        slotIdentitiesChanged();
    }

    const QSet<QString> &emails()
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
