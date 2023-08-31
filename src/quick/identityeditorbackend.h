// SPDX-FileCopyrightText: 2023 Claudio Cambra <claudio.cambra@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QObject>

#include "identity.h"
#include "identitymanager.h"

namespace KIdentityManagementQuick
{
class IdentityEditorBackend : public QObject
{
    Q_OBJECT

    Q_PROPERTY(Mode mode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(KIdentityManagement::Identity identity READ identity WRITE setIdentity NOTIFY identityChanged NOTIFY modeChanged)
    Q_PROPERTY(uint identityUoid READ identityUoid WRITE setIdentityUoid NOTIFY identityChanged)

public:
    enum Mode { CreateMode, EditMode };
    Q_ENUM(Mode);

    explicit IdentityEditorBackend() = default;

    Q_REQUIRED_RESULT Mode mode() const;
    void setMode(Mode mode);

    Q_REQUIRED_RESULT KIdentityManagement::Identity identity() const;
    void setIdentity(const KIdentityManagement::Identity &identity);

    Q_REQUIRED_RESULT uint identityUoid() const;
    void setIdentityUoid(uint identityUoid);

    Q_INVOKABLE void saveIdentity(const KIdentityManagement::Identity &modifiedIdentity);

    Q_INVOKABLE void addEmailAlias(const QString &alias);
    Q_INVOKABLE void modifyEmailAlias(const QString &originalAlias, const QString &modifiedAlias);
    Q_INVOKABLE void removeEmailAlias(const QString &alias);

Q_SIGNALS:
    void modeChanged();
    void identityChanged();

private:
    KIdentityManagement::IdentityManager *const m_identityManager = KIdentityManagement::IdentityManager::self();
    KIdentityManagement::Identity m_identity;
    Mode m_mode = CreateMode;
};
}
