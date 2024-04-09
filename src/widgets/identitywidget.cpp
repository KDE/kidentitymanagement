// SPDX-FileCopyrightText: 2024 Laurent Montel <montel@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "identitywidget.h"
#include "identitytreeview.h"
#include <KLocalizedString>
#include <QMenu>
#include <QVBoxLayout>
using namespace Qt::Literals::StringLiterals;
using namespace KIdentityManagementWidgets;
IdentityWidget::IdentityWidget(QWidget *parent)
    : QWidget{parent}
{
    mUi = new Ui::IdentityWidget();
    mUi->setupUi(this);
    // auto mainLayout = new QVBoxLayout(this);
    // mainLayout->setObjectName("mainLayout"_L1);
    // mainLayout->setContentsMargins({});

    // mIdentityTreeView->setObjectName("mIdentityTreeView"_L1);
    // mainLayout->addWidget(mIdentityTreeView);
    // connect(mIdentityTreeView, &QTreeView::customContextMenuRequested, this, &IdentityWidget::slotCustomContextMenuRequested);
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
#if 0
    const QModelIndex index = mIdentityTreeView->indexAt(pos);
    QMenu menu(this);
    menu.addAction(QIcon::fromTheme(QStringLiteral("list-add")), i18n("Add..."), this, &IdentityWidget::slotNewIdentity);
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
    menu.exec(pos);
#endif
}

void IdentityWidget::slotNewIdentity()
{
    // TODO
}

void IdentityWidget::updateButtons()
{
    // TODO
}

#include "moc_identitywidget.cpp"
