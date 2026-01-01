/*
  SPDX-FileCopyrightText: 2024-2026 Laurent Montel <montel@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "identitytreedelegate.h"
#include "identitytreemodel.h"
#include <KLineEditEventHandler>
#include <QLineEdit>
using namespace KIdentityManagementWidgets;
IdentityTreeDelegate::IdentityTreeDelegate(QObject *parent)
    : QStyledItemDelegate{parent}
{
}

IdentityTreeDelegate::~IdentityTreeDelegate() = default;

QWidget *IdentityTreeDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    Q_UNUSED(option);
    auto *lineEdit = new QLineEdit(parent);
    KLineEditEventHandler::catchReturnKey(lineEdit);
    auto *that = const_cast<IdentityTreeDelegate *>(this);
    connect(lineEdit, &QLineEdit::editingFinished, this, [this, that]() {
        Q_EMIT that->commitData(mLineEdit);
        Q_EMIT that->closeEditor(mLineEdit);
    });
    const_cast<IdentityTreeDelegate *>(this)->mLineEdit = lineEdit;
    return lineEdit;
}

void IdentityTreeDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    auto identityNameIndex = index.model()->index(index.row(), KIdentityManagementCore::IdentityTreeModel::IdentityNameRole);
    qobject_cast<QLineEdit *>(editor)->setText(identityNameIndex.data().toString());
}

void IdentityTreeDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    auto identityNameIndex = index.model()->index(index.row(), KIdentityManagementCore::IdentityTreeModel::IdentityNameRole);
    model->setData(identityNameIndex, qobject_cast<QLineEdit *>(editor)->text());
}

void IdentityTreeDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex & /*index*/) const
{
    editor->setGeometry(option.rect);
}

#include "moc_identitytreedelegate.cpp"
