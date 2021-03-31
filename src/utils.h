/*
  SPDX-FileCopyrightText: 2014 Sandro Knauß <knauss@kolabsys.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kidentitymanagement_export.h"

#include <QSet>
#include <QString>

namespace KIdentityManagement
{
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

