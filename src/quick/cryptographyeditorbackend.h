// SPDX-FileCopyrightText: 2023 Claudio Cambra <claudio.cambra@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include "kidentitymanagementquick_export.h"

#include <QModelIndex>
#include <QObject>

#include <KIdentityManagementCore/CryptographyBackendInterface>
#include <KIdentityManagementCore/Identity>
#include <KIdentityManagementCore/KeyListModelInterface>

namespace KIdentityManagementCore
{
namespace Quick
{

/**
 * @brief The CryptographyEditorBackend class.
 *
 * Used by the QML interface to access cryptography-related data.
 * Note that since the CryptographyBackendInterface is an abstract class,
 * it is not accessible from QML. You will need to instantiate it in C++
 * and then feed it into the QML side of the editor with a valid backend
 * already set!
 */
class KIDENTITYMANAGEMENTQUICK_EXPORT CryptographyEditorBackend : public QObject
{
    Q_OBJECT

    Q_PROPERTY(Identity identity READ identity WRITE setIdentity NOTIFY identityChanged)
    Q_PROPERTY(QAbstractItemModel *openPgpKeyListModel READ openPgpKeyListModel NOTIFY openPgpKeyListModelChanged)
    Q_PROPERTY(QAbstractItemModel *smimeKeyListModel READ smimeKeyListModel NOTIFY smimeKeyListModelChanged)

public:
    explicit CryptographyEditorBackend(QObject *parent = nullptr, const CryptographyBackendInterfacePtr &backend = {});

    CryptographyBackendInterfacePtr cryptographyBackend() const;
    void setCryptographyBackend(const CryptographyBackendInterfacePtr &cryptographyBackend);

    QAbstractItemModel *openPgpKeyListModel() const;
    QAbstractItemModel *smimeKeyListModel() const;

    Identity identity() const;
    void setIdentity(const Identity &identity);

    Q_INVOKABLE QModelIndex indexForIdentity(QAbstractItemModel *model, const Identity &identity, const KeyUseTypes::KeyUse keyUse);

    Q_INVOKABLE static QString stringFromKeyByteArray(const QByteArray &key);

Q_SIGNALS:
    void cryptographyBackendChanged();
    void openPgpKeyListModelChanged();
    void smimeKeyListModelChanged();
    void identityChanged();

private:
    CryptographyBackendInterfacePtr m_cryptoBackend;
};

}
}
