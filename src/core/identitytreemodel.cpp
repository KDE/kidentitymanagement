// SPDX-FileCopyrightText: 2021 Carl Schwan <carlschwan@kde.org>
// SPDX-FileCopyrightText: 2023 Claudio Cambra <claudio.cambra@kde.org>
// SPDX-FileCopyrightText: 2024-2026 Laurent Montel <montel@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "identitytreemodel.h"
using namespace Qt::Literals::StringLiterals;

#include "kidentitymanagementcore_debug.h"
#include <KLocalizedString>
#include <QFont>

#include "identity.h"

using namespace KIdentityManagementCore;
IdentityTreeModel::IdentityTreeModel(IdentityManager *manager, QObject *parent)
    : QAbstractListModel(parent)
    , mIdentityManager(manager)
{
    connect(mIdentityManager, &IdentityManager::needToReloadIdentitySettings, this, &IdentityTreeModel::reloadUoidList);
    connect(mIdentityManager, &IdentityManager::identitiesWereChanged, this, &IdentityTreeModel::reloadUoidList);
    connect(mIdentityManager, &IdentityManager::deleted, this, &IdentityTreeModel::reloadUoidList);
    connect(mIdentityManager, &IdentityManager::added, this, &IdentityTreeModel::reloadUoidList);
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
        if (static_cast<IdentityRoles>(index.column()) == DisplayIdentityNameRole) {
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
        return identity.identityName();
    case DisplayIdentityNameRole:
        return generateIdentityName(identity);
    case DefaultRole:
        return identity.isDefault();
    case ActivitiesRole:
        return identity.activities();
    case EnabledActivitiesRole:
        return identity.enabledActivities();
    }

    return {};
}

QString IdentityTreeModel::generateIdentityName(const Identity &identity) const
{
    QString str = identity.identityName();
    if (mShowDefault && identity.isDefault()) {
        str += u' ' + i18nc("Default identity", " (default)");
    }
    return str;
}

KIdentityManagementCore::IdentityManager *IdentityTreeModel::identityManager() const
{
    return mIdentityManager;
}

int IdentityTreeModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) // flat model
        return 0;
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
        case DisplayIdentityNameRole:
            return i18n("Identity Name");
        case FullEmailRole:
        case EmailRole:
            return i18n("Email Address");
        case UoidRole:
        case DefaultRole:
        case IdentityNameRole:
        case ActivitiesRole:
        case EnabledActivitiesRole:
            return {};
        }
    }
    return {};
}

Qt::ItemFlags IdentityTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    if (static_cast<IdentityRoles>(index.column()) == DisplayIdentityNameRole) {
        return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
    }
    return QAbstractItemModel::flags(index);
}

bool IdentityTreeModel::setData(const QModelIndex &modelIndex, const QVariant &value, int role)
{
    if (!modelIndex.isValid()) {
        qCWarning(KIDENTITYMANAGEMENT_LOG) << "ERROR: invalid index";
        return false;
    }
    if (role != Qt::EditRole) {
        return false;
    }
    const int idx = modelIndex.row();
    auto &identity = mIdentityManager->modifyIdentityForUoid(mIdentitiesUoid[idx]);
    switch (static_cast<IdentityRoles>(modelIndex.column())) {
    case IdentityNameRole: {
        const QModelIndex newIndex = index(modelIndex.row(), IdentityNameRole);
        Q_EMIT dataChanged(newIndex, newIndex);
        identity.setIdentityName(value.toString());
        mIdentityManager->saveIdentity(identity);
        return true;
    }
    case DefaultRole: {
        const QModelIndex newIndex = index(modelIndex.row(), UoidRole);
        mIdentityManager->setAsDefault(newIndex.data().toInt());
        Q_EMIT dataChanged(modelIndex, modelIndex);
        return true;
    }
    default:
        break;
    }
    return false;
}

void IdentityTreeModel::removeIdentities(const QStringList &identitiesName)
{
    for (const QString &name : identitiesName) {
        mIdentityManager->removeIdentity(name);
    }
}

#include "moc_identitytreemodel.cpp"
