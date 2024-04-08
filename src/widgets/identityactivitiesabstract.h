// SPDX-FileCopyrightText: 2024 Laurent Montel <montel@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#pragma once

#include "kidentitymanagementwidgets_export.h"
#include <QObject>

namespace KIdentityManagementWidgets
{
class KIDENTITYMANAGEMENTWIDGETS_EXPORT IdentityActivitiesAbstract : public QObject
{
    Q_OBJECT
public:
    explicit IdentityActivitiesAbstract(QObject *parent = nullptr);
    ~IdentityActivitiesAbstract() override;

    virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const = 0;
};
}
