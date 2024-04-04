// SPDX-FileCopyrightText: 2024 Laurent Montel <montel@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include <KIdentityManagementCore/IdentityManager>
#include <KIdentityManagementWidgets/IdentityCombo>
#include <QApplication>
#include <QVBoxLayout>
#include <QWidget>

class IdentityComboboxWidget : public QWidget
{
public:
    explicit IdentityComboboxWidget(QWidget *parent = nullptr);
};

IdentityComboboxWidget::IdentityComboboxWidget(QWidget *parent)
    : QWidget(parent)
{
    auto mainLayout = new QVBoxLayout(this);
    auto combobox = new KIdentityManagementWidgets::IdentityCombo(KIdentityManagementCore::IdentityManager::self(), this);
    mainLayout->addWidget(combobox);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    auto w = new IdentityComboboxWidget;
    w->show();
    return app.exec();
}
