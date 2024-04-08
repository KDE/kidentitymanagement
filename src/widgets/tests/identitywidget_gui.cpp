// SPDX-FileCopyrightText: 2024 Laurent Montel <montel@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include <KIdentityManagementCore/IdentityManager>
#include <KIdentityManagementWidgets/IdentityWidget>
#include <QApplication>
#include <QVBoxLayout>
#include <QWidget>

class IdentityWidget : public QWidget
{
public:
    explicit IdentityWidget(QWidget *parent = nullptr);
    ~IdentityWidget() override = default;
};

IdentityWidget::IdentityWidget(QWidget *parent)
    : QWidget(parent)
{
    auto mainLayout = new QVBoxLayout(this);
    auto treeview = new KIdentityManagementWidgets::IdentityWidget(this);
    mainLayout->addWidget(treeview);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    auto w = new IdentityWidget;
    w->show();
    return app.exec();
}
