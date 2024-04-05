// SPDX-FileCopyrightText: 2021 Carl Schwan <carlschwan@kde.org>
// SPDX-FileCopyrightText: 2023 Claudio Cambra <claudio.cambra@kde.org>
// SPDX-FileCopyrightText: 2024 Laurent Montel <montel@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "identitytablemodel.h"

#include <KLocalizedString>

#include "identity.h"

using namespace KIdentityManagementWidgets;
using namespace KIdentityManagementCore;
IdentityTableModel::IdentityTableModel(QObject *parent)
    : QAbstractListModel(parent)
    , mIdentityManager(IdentityManager::self())
{
    connect(mIdentityManager, &IdentityManager::needToReloadIdentitySettings, this, &IdentityTableModel::reloadUoidList);
    connect(mIdentityManager, &IdentityManager::identitiesWereChanged, this, &IdentityTableModel::reloadUoidList);
    reloadUoidList();
}

void IdentityTableModel::reloadUoidList()
{
    beginResetModel();
    mIdentitiesUoid.clear();
    for (const auto &identity : *mIdentityManager) {
        mIdentitiesUoid << identity.uoid();
    }
    endResetModel();
}

IdentityTableModel::~IdentityTableModel() = default;

int IdentityTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    constexpr int nbCol = static_cast<int>(IdentityRoles::LastColumn) + 1;
    return nbCol;
}

QVariant IdentityTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    const auto &identity = mIdentityManager->modifyIdentityForUoid(mIdentitiesUoid[index.row()]);
    if (role == Qt::ToolTipRole) {
        return identity.primaryEmailAddress();
    }
    if (role != Qt::DisplayRole) {
        return {};
    }
    switch (static_cast<IdentityRoles>(index.column())) {
    case EmailRole:
        return identity.primaryEmailAddress();
    case UoidRole:
        return identity.uoid();
    case IdentityNameRole:
        return generateIdentityName(identity);
    case DefaultRole:
        return identity.isDefault();
    }

    return {};
}

QString IdentityTableModel::generateIdentityName(const Identity &identity) const
{
    QString str = identity.identityName();
    if (identity.isDefault()) {
        str += QLatin1Char(' ') + i18nc("Default identity", " (default)");
    }
    return str;
}

int IdentityTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return mIdentitiesUoid.count();
}

void IdentityTableModel::setShowDefault(bool show)
{
    mShowDefault = show;
}

uint IdentityTableModel::identityUoid(int index) const
{
    return mIdentitiesUoid.at(index);
}

int IdentityTableModel::uoidIndex(int uoid) const
{
    return mIdentitiesUoid.indexOf(uoid);
}

#include "moc_identitytablemodel.cpp"
