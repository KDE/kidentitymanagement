// SPDX-FileCopyrightText: 2021 Carl Schwan <carlschwan@kde.org>
// SPDX-FileCopyrightText: 2023 Claudio Cambra <claudio.cambra@kde.org>
// SPDX-FileCopyrightText: 2024 Laurent Montel <montel@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "identitytreemodel.h"
#include <KLocalizedString>
#include <QFont>

#include "identity.h"

using namespace KIdentityManagementWidgets;
using namespace KIdentityManagementCore;
IdentityTreeModel::IdentityTreeModel(QObject *parent)
    : QAbstractListModel(parent)
    , mIdentityManager(IdentityManager::self())
{
    connect(mIdentityManager, &IdentityManager::needToReloadIdentitySettings, this, &IdentityTreeModel::reloadUoidList);
    connect(mIdentityManager, &IdentityManager::identitiesWereChanged, this, &IdentityTreeModel::reloadUoidList);
    reloadUoidList();
}

void IdentityTreeModel::reloadUoidList()
{
    beginResetModel();
    mIdentitiesUoid.clear();
    for (const auto &identity : *mIdentityManager) {
        mIdentitiesUoid << identity.uoid();
    }
    endResetModel();
}

IdentityTreeModel::~IdentityTreeModel() = default;

int IdentityTreeModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    constexpr int nbCol = static_cast<int>(IdentityRoles::LastColumn) + 1;
    return nbCol;
}

QVariant IdentityTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    const auto &identity = mIdentityManager->modifyIdentityForUoid(mIdentitiesUoid[index.row()]);
    if (role == Qt::ToolTipRole) {
        return identity.primaryEmailAddress();
    }
    if (role == Qt::FontRole) {
        if (static_cast<IdentityRoles>(index.column()) == IdentityNameRole) {
            if (identity.isDefault()) {
                QFont f;
                f.setBold(true);
                return f;
            }
        }
    }
    if (role != Qt::DisplayRole) {
        return {};
    }
    switch (static_cast<IdentityRoles>(index.column())) {
    case FullEmailRole:
        return identity.fullEmailAddr();
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

QString IdentityTreeModel::generateIdentityName(const Identity &identity) const
{
    QString str = identity.identityName();
    if (mShowDefault && identity.isDefault()) {
        str += QLatin1Char(' ') + i18nc("Default identity", " (default)");
    }
    return str;
}

int IdentityTreeModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return mIdentitiesUoid.count();
}

void IdentityTreeModel::setShowDefault(bool show)
{
    mShowDefault = show;
}

uint IdentityTreeModel::identityUoid(int index) const
{
    return mIdentitiesUoid.at(index);
}

int IdentityTreeModel::uoidIndex(int uoid) const
{
    return mIdentitiesUoid.indexOf(uoid);
}

QVariant IdentityTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (static_cast<IdentityRoles>(section)) {
        case IdentityNameRole:
            return i18n("Identity Name");
        case FullEmailRole:
        case EmailRole:
            return i18n("Email Address");
        case UoidRole:
        case DefaultRole:
            return {};
        }
    }
    return {};
}

#include "moc_identitytreemodel.cpp"
