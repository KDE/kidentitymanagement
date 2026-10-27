// SPDX-FileCopyrightText: 2023 Claudio Cambra <claudio.cambra@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include "kidentitymanagementquick_export.h"

#include <QModelIndex>
#include <QObject>

#include <KIdentityManagementCore/Identity>
#include <KIdentityManagementQuick/CryptographyBackendInterface>
#include <KIdentityManagementQuick/KeyListModelInterface>

namespace KIdentityManagementQuick
{
/*!
 * \class KIdentityManagementQuick::CryptographyEditorBackend
 * \inmodule KIdentityManagementQuick
 * \inheaderfile KIdentityManagementQuick/CryptographyEditorBackend
 *
 * \brief The CryptographyEditorBackend class.
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

    /*!
     * \property KIdentityManagementQuick::CryptographyEditorBackend::identity
     * \brief the edited identity.
     */
    Q_PROPERTY(KIdentityManagementCore::Identity identity READ identity WRITE setIdentity NOTIFY identityChanged)
    /*!
     * \property KIdentityManagementQuick::CryptographyEditorBackend::openPgpKeyListModel
     * \brief the model listing the OpenPGP keys.
     */
    Q_PROPERTY(QAbstractItemModel *openPgpKeyListModel READ openPgpKeyListModel NOTIFY openPgpKeyListModelChanged)
    /*!
     * \property KIdentityManagementQuick::CryptographyEditorBackend::smimeKeyListModel
     * \brief the model listing the S/MIME keys.
     */
    Q_PROPERTY(QAbstractItemModel *smimeKeyListModel READ smimeKeyListModel NOTIFY smimeKeyListModelChanged)

public:
    /*!
     * Constructs a cryptography editor backend with the given \a parent, using \a backend to access the keys.
     */
    explicit CryptographyEditorBackend(QObject *parent = nullptr, const CryptographyBackendInterfacePtr &backend = {});

    /*!
     * Returns the cryptography backend.
     */
    CryptographyBackendInterfacePtr cryptographyBackend() const;
    /*!
     * Sets the cryptography backend to \a cryptographyBackend.
     */
    void setCryptographyBackend(const CryptographyBackendInterfacePtr &cryptographyBackend);

    /*!
     * Returns the model listing the OpenPGP keys.
     */
    QAbstractItemModel *openPgpKeyListModel() const;
    /*!
     * Returns the model listing the S/MIME keys.
     */
    QAbstractItemModel *smimeKeyListModel() const;

    /*!
     * Returns the edited identity.
     */
    [[nodiscard]] KIdentityManagementCore::Identity identity() const;
    /*!
     * Sets the edited identity to \a identity.
     */
    void setIdentity(const KIdentityManagementCore::Identity &identity);

    /*!
     * Returns the index in \a model of the key used by \a identity for \a keyUse.
     * \a model must implement KeyListModelInterface.
     */
    Q_INVOKABLE QModelIndex indexForIdentity(QAbstractItemModel *model,
                                             const KIdentityManagementCore::Identity &identity,
                                             const KIdentityManagementQuick::KeyUseTypes::KeyUse keyUse);

    /*!
     * Returns \a key converted to a string.
     */
    Q_INVOKABLE static QString stringFromKeyByteArray(const QByteArray &key);

Q_SIGNALS:
    /*!
     * Emitted when the cryptography backend changes.
     */
    void cryptographyBackendChanged();
    /*!
     * Emitted when the OpenPGP key list model changes.
     */
    void openPgpKeyListModelChanged();
    /*!
     * Emitted when the S/MIME key list model changes.
     */
    void smimeKeyListModelChanged();
    /*!
     * Emitted when the edited identity changes.
     */
    void identityChanged();

private:
    CryptographyBackendInterfacePtr mCryptoBackend;
};

}
