// SPDX-FileCopyrightText: 2024 Laurent Montel <montel@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "identitywidget.h"
#include "identitytreeview.h"
#include <QVBoxLayout>
using namespace Qt::Literals::StringLiterals;
using namespace KIdentityManagementWidgets;
IdentityWidget::IdentityWidget(QWidget *parent)
    : QWidget{parent}
    , mIdentityTreeView(new IdentityTreeView(this))
{
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setObjectName("mainLayout"_L1);

    mIdentityTreeView->setObjectName("mIdentityTreeView"_L1);
    mainLayout->addWidget(mIdentityTreeView);
}

IdentityWidget::~IdentityWidget() = default;

#include "moc_identitywidget.cpp"
