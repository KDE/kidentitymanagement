// SPDX-FileCopyrightText: 2024-2026 Laurent Montel <montel@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include <KIdentityManagementWidgets/IdentityTreeView>
#include <QApplication>
#include <QVBoxLayout>
#include <QWidget>

class IdentityTreeWidget : public QWidget
{
public:
    explicit IdentityTreeWidget(QWidget *parent = nullptr);
    ~IdentityTreeWidget() override = default;
};

IdentityTreeWidget::IdentityTreeWidget(QWidget *parent)
    : QWidget(parent)
{
    auto mainLayout = new QVBoxLayout(this);
    auto treeview = new KIdentityManagementWidgets::IdentityTreeView(this);
    mainLayout->addWidget(treeview);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    auto w = new IdentityTreeWidget;
    w->show();
    return app.exec();
}
