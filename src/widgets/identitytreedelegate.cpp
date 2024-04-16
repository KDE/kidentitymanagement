/*
  SPDX-FileCopyrightText: 2024 Laurent Montel <montel@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "identitytreedelegate.h"
#include "identitytreemodel.h"
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
    auto loggingIndex = index.model()->index(index.row(), KIdentityManagementCore::IdentityTreeModel::DisplayIdentityNameRole);
    qobject_cast<QLineEdit *>(editor)->setText(loggingIndex.data().toString());
}

void IdentityTreeDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    auto loggingIndex = index.model()->index(index.row(), KIdentityManagementCore::IdentityTreeModel::DisplayIdentityNameRole);
    model->setData(loggingIndex, qobject_cast<QLineEdit *>(editor)->text());
}

void IdentityTreeDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex & /*index*/) const
{
    editor->setGeometry(option.rect);
}

#include "moc_identitytreedelegate.cpp"
