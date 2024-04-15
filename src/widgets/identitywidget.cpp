// SPDX-FileCopyrightText: 2024 Laurent Montel <montel@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "identitywidget.h"
#include "identitytreeview.h"
#include <KLocalizedString>
#include <QMenu>
#include <QPushButton>
#include <QVBoxLayout>
using namespace Qt::Literals::StringLiterals;
using namespace KIdentityManagementWidgets;
IdentityWidget::IdentityWidget(QWidget *parent)
    : QWidget{parent}
    , mUi(new Ui::IdentityWidget())
{
    mUi->setupUi(this);
    // connect(mIdentityTreeView, &QTreeView::customContextMenuRequested, this, &IdentityWidget::slotCustomContextMenuRequested);

    connect(mUi->mButtonAdd, &QPushButton::clicked, this, &IdentityWidget::slotNewIdentity);
    connect(mUi->mModifyButton, &QPushButton::clicked, this, &IdentityWidget::slotModifyIdentity);
    connect(mUi->mRenameButton, &QPushButton::clicked, this, &IdentityWidget::slotRenameIdentity);
    connect(mUi->mRemoveButton, &QPushButton::clicked, this, &IdentityWidget::slotRemoveIdentity);
    connect(mUi->mSetAsDefaultButton, &QPushButton::clicked, this, &IdentityWidget::slotSetAsDefault);
}

IdentityWidget::~IdentityWidget()
{
    delete mUi;
}

void IdentityWidget::setIdentityActivitiesAbstract(KIdentityManagementCore::IdentityActivitiesAbstract *newIdentityActivitiesAbstract)
{
    mUi->mIdentityView->setIdentityActivitiesAbstract(newIdentityActivitiesAbstract);
}

KIdentityManagementCore::IdentityActivitiesAbstract *IdentityWidget::identityActivitiesAbstract() const
{
    return mUi->mIdentityView->identityActivitiesAbstract();
}

void IdentityWidget::slotCustomContextMenuRequested(const QPoint &pos)
{
    const QModelIndex index = mUi->mIdentityView->indexAt(pos);
    QMenu menu(this);
    menu.addAction(QIcon::fromTheme(QStringLiteral("list-add")), i18n("Add..."), this, &IdentityWidget::slotNewIdentity);
#if 0
    if (item) {
        menu.addAction(QIcon::fromTheme(QStringLiteral("document-edit")), i18n("Modify..."), this, &IdentityPage::slotModifyIdentity);
        menu.addAction(QIcon::fromTheme(QStringLiteral("edit-rename")), i18n("Rename"), this, &IdentityPage::slotRenameIdentity);
        if (mIPage.mIdentityList->topLevelItemCount() > 1) {
            menu.addAction(QIcon::fromTheme(QStringLiteral("list-remove")), i18n("Remove"), this, &IdentityPage::slotRemoveIdentity);
        }
        if (!item->identity().isDefault()) {
            menu.addSeparator();
            menu.addAction(i18n("Set as Default"), this, &IdentityPage::slotSetAsDefault);
        }
    }
#endif
    menu.exec(pos);
}

void IdentityWidget::slotNewIdentity()
{
    // TODO
}

void IdentityWidget::updateButtons()
{
    // TODO
}

void IdentityWidget::slotModifyIdentity()
{
    // TODO
}

void IdentityWidget::slotRenameIdentity()
{
    // TODO
}

void IdentityWidget::slotRemoveIdentity()
{
    // TODO
}

void IdentityWidget::slotSetAsDefault()
{
#if 0
    IdentityListViewItem *item = nullptr;
    if (!mIPage.mIdentityList->selectedItems().isEmpty()) {
        item = dynamic_cast<IdentityListViewItem *>(mIPage.mIdentityList->selectedItems().first());
    }
    if (!item) {
        return;
    }

    mIdentityManager->setAsDefault(item->identity().uoid());
    refreshList();
    mUi->mSetAsDefaultButton->setEnabled(false);
#endif
    // TODO
}

#include "moc_identitywidget.cpp"
