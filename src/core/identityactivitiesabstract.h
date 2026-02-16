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
 *
 * \brief The IdentityActivitiesAbstract class
 * \author Laurent Montel <montel@kde.org>
 */
class KIDENTITYMANAGEMENTCORE_EXPORT IdentityActivitiesAbstract : public QObject
{
    Q_OBJECT
public:
    /*!
     * Constructor
     * \param parent the parent object
     */
    explicit IdentityActivitiesAbstract(QObject *parent = nullptr);
    /*!
     * Destructor
     */
    ~IdentityActivitiesAbstract() override;

    /*!
     * Determines whether a row should be accepted based on the given activities
     * \param activities the list of activities to filter against
     * \return true if the row should be accepted, false otherwise
     */
    [[nodiscard]] virtual bool filterAcceptsRow(const QStringList &activities) const = 0;

    /*!
     * Returns whether activities are supported
     * \return true if activities are supported, false otherwise
     */
    [[nodiscard]] virtual bool hasActivitySupport() const = 0;

    /*!
     * Returns the current activity
     * \return the current activity identifier
     */
    [[nodiscard]] virtual QString currentActivity() const = 0;

Q_SIGNALS:
    /*! Emitted when the activities change */
    void activitiesChanged();
};
}
