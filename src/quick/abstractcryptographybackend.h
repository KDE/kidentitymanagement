// SPDX-FileCopyrightText: 2023 Claudio Cambra <claudio.cambra@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QObject>

namespace KIdentityManagement
{
namespace Quick
{

class AbstractKeyListModel;

class AbstractCryptographyBackend : public QObject
{
    Q_OBJECT

    Q_PROPERTY(AbstractKeyListModel *openPgpKeyListModel READ openPgpKeyListModel CONSTANT)
    Q_PROPERTY(AbstractKeyListModel *smimeKeyListModel READ smimeKeyListModel CONSTANT)

public:
    explicit AbstractCryptographyBackend(QObject *parent = nullptr);

    /**
     * A list model providing a list of OpenPGP keys.
     */
    virtual AbstractKeyListModel *openPgpKeyListModel() const;

    /**
     * A list model providing a list of S/MIME keys.
     */
    virtual AbstractKeyListModel *smimeKeyListModel() const;
};

}
}
