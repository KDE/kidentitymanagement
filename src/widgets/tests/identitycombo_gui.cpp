// SPDX-FileCopyrightText: 2024-2026 Laurent Montel <montel@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include <KIdentityManagementCore/IdentityManager>
using namespace Qt::Literals::StringLiterals;

#include <KIdentityManagementWidgets/IdentityCombo>
#include <QApplication>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

class IdentityComboboxWidget : public QWidget
{
public:
    explicit IdentityComboboxWidget(QWidget *parent = nullptr);
    ~IdentityComboboxWidget() override = default;
};

IdentityComboboxWidget::IdentityComboboxWidget(QWidget *parent)
    : QWidget(parent)
{
    auto mainLayout = new QVBoxLayout(this);
    auto combobox = new KIdentityManagementWidgets::IdentityCombo(KIdentityManagementCore::IdentityManager::self(), this);
    mainLayout->addWidget(combobox);

    auto label = new QLabel(this);
    connect(combobox, &KIdentityManagementWidgets::IdentityCombo::identityChanged, this, [label](KIdentityManagementCore::Identity::Id id) {
        label->setText(QString::number(id));
    });
    mainLayout->addWidget(label);

    auto labelIdentity = new QLabel(this);
    connect(combobox,
            &KIdentityManagementWidgets::IdentityCombo::identityChanged,
            this,
            [combobox, labelIdentity]([[maybe_unused]] KIdentityManagementCore::Identity::Id id) {
                labelIdentity->setText(combobox->currentIdentityName());
            });
    mainLayout->addWidget(labelIdentity);

    {
        auto hbox = new QHBoxLayout;
        auto identityNameLineEdit = new QLineEdit(this);

        hbox->addWidget(new QLabel(u"set identity name:"_s, this));
        hbox->addWidget(identityNameLineEdit);

        auto identityNameButton = new QPushButton(u"Apply"_s, this);
        hbox->addWidget(identityNameButton);

        connect(identityNameButton, &QPushButton::clicked, this, [identityNameLineEdit, combobox]() {
            combobox->setCurrentIdentity(identityNameLineEdit->text());
        });

        mainLayout->addLayout(hbox);
    }

    {
        auto hbox = new QHBoxLayout;
        auto identityNameLineEdit = new QLineEdit(this);

        hbox->addWidget(new QLabel(u"set identity identifier:"_s, this));
        hbox->addWidget(identityNameLineEdit);

        auto identityNameButton = new QPushButton(u"Apply"_s, this);
        hbox->addWidget(identityNameButton);

        connect(identityNameButton, &QPushButton::clicked, this, [identityNameLineEdit, combobox]() {
            combobox->setCurrentIdentity(identityNameLineEdit->text().toInt());
        });

        mainLayout->addLayout(hbox);
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    auto w = new IdentityComboboxWidget;
    w->show();
    return app.exec();
}
