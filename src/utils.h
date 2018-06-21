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

#ifndef KPIMIDENTITES_UTILS_H
#define KPIMIDENTITES_UTILS_H

#include "kidentitymanagement_export.h"

#include <QString>
#include <QSet>

namespace KIdentityManagement {
/*
* Very fast version of IdentityManager::thatIsMe, that is using an internal cache (allEmails)
* - make sure that only an email address is used as parameter and NO name <email>
* - emails are tested with email.toLower(), so no need to lower them before.
*/
Q_REQUIRED_RESULT KIDENTITYMANAGEMENT_EXPORT bool thatIsMe(const QString &email);

/*
* Very fast version of IdentityManager::allEmails , that is using an internal cache.
* The cache is updated with IdentityManager::changed signal.
* All email addresses + alias of the identities. The email addresses are all lowered.
*/
Q_REQUIRED_RESULT KIDENTITYMANAGEMENT_EXPORT const QSet<QString> &allEmails();
}

#endif
