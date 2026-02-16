/*
  SPDX-FileCopyrightText: 2014 Sandro Knauß <knauss@kolabsys.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kidentitymanagementcore_export.h"

#include <QSet>
#include <QString>

namespace KIdentityManagementCore
{
/*! Very fast version of IdentityManager::thatIsMe, that uses an internal cache (allEmails)
 *  \note Make sure that only an email address is used as parameter and NO name <email>
 *  \note Emails are tested with email.toLower(), so no need to lower them before.
 *  \param email the email address to check
 *  \return true if this email address matches one of our identities, false otherwise
 */
[[nodiscard]] KIDENTITYMANAGEMENTCORE_EXPORT bool thatIsMe(const QString &email);

/*! Very fast version of IdentityManager::allEmails, that uses an internal cache.
 *  The cache is updated with IdentityManager::changed signal.
 *  \return All email addresses and aliases of the identities (all lowered).
 */
[[nodiscard]] KIDENTITYMANAGEMENTCORE_EXPORT const QSet<QString> &allEmails();
}
