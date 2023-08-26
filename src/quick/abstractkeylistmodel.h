// SPDX-FileCopyrightText: 2023 Claudio Cambra <claudio.cambra@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QAbstractListModel>
#include <QHash>

namespace KIdentityManagement
{
namespace Quick
{

class AbstractKeyListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles { KeyIdentifierRole = Qt::UserRole, KeyByteArrayRole };
    Q_ENUM(Roles)

    explicit AbstractKeyListModel(QObject *parent = nullptr);

    QHash<int, QByteArray> roleNames() const override;
};

}
}
