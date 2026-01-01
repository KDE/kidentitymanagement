// SPDX-FileCopyrightText: 2024-2026 Laurent Montel <montel@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#pragma once

#include "kidentitymanagementcore_export.h"
#include <QObject>

namespace KIdentityManagementCore
{
/*!
 * \class KIdentityManagementCore::IdentityActivitiesAbstract
 * \inmodule KIdentityManagementCore
 * \inheaderfile KIdentityManagementCore/IdentityActivitiesAbstract
 */
class KIDENTITYMANAGEMENTCORE_EXPORT IdentityActivitiesAbstract : public QObject
{
    Q_OBJECT
public:
    /*!
     */
    explicit IdentityActivitiesAbstract(QObject *parent = nullptr);
    /*!
     */
    ~IdentityActivitiesAbstract() override;

    /*!
     */
    [[nodiscard]] virtual bool filterAcceptsRow(const QStringList &activities) const = 0;

    /*!
     */
    [[nodiscard]] virtual bool hasActivitySupport() const = 0;

    /*!
     */
    [[nodiscard]] virtual QString currentActivity() const = 0;

Q_SIGNALS:
    /*!
     */
    void activitiesChanged();
};
}
