// SPDX-FileCopyrightText: 2023 Claudio Cambra <claudio.cambra@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import Qt.labs.platform 1.1

import org.kde.kirigami 2.19 as Kirigami
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm
import org.kde.kidentitymanagement 1.0

MobileForm.FormCard {
    id: root

    required property var identity
    property alias toAddEmailText: toAddEmail.text

    function isNotEmptyStr(str) {
        return str.trim().length > 0;
    }

    Layout.fillWidth: true
    Layout.topMargin: Kirigami.Units.largeSpacing

    contentItem: ColumnLayout {
        spacing: 0

        MobileForm.FormCardHeader {
            title: i18n("Identity")
        }

        MobileForm.FormTextFieldDelegate {
            id: nameDelegate
            Layout.fillWidth: true
            label: i18n("Your name")
            text: root.identity.fullName
            onTextChanged: root.identity.fullName = text
        }

        MobileForm.FormTextFieldDelegate {
            id: emailDelegate
            Layout.fillWidth: true
            label: i18n("Email address")
            text: root.identity.primaryEmailAddress
            onTextChanged: root.identity.primaryEmailAddress = text
            inputMethodHints: Qt.ImhEmailCharactersOnly
        }

        MobileForm.FormCardHeader {
            title: i18n("E-mail aliases")
        }

        Repeater {
            id: emailAliasesRepeater

            readonly property var emailAliases: root.identity.emailAliases

            model: emailAliases

            delegate: MobileForm.AbstractFormDelegate {
                id: emailRow

                Layout.fillWidth: true

                contentItem: RowLayout {
                    QQC2.TextField {
                        id: emailAliasField
                        Layout.fillWidth: true
                        text: modelData
                        inputMethodHints: Qt.ImhEmailCharactersOnly
                        onEditingFinished: {
                            emailAliasesRepeater.emailAliases[model.index] = text;
                            identity.emailAliases = emailAliasesRepeater.emailAliases;
                        }
                    }

                    QQC2.Button {
                        icon.name: "list-remove"
                        implicitWidth: implicitHeight
                        QQC2.ToolTip {
                            text: i18n("Remove email alias")
                        }
                        onClicked: {
                            let emailAliases = emailAliasesRepeater.emailAliases;
                            emailAliases = Array.from(emailAliases).filter(email => email !== modelData);
                            identity.emailAliases = emailAliases;
                        }
                    }
                }
            }
        }
        MobileForm.AbstractFormDelegate {
            Layout.fillWidth: true

            contentItem: RowLayout {
                QQC2.TextField {
                    id: toAddEmail
                    Layout.fillWidth: true
                    placeholderText: i18n("user@example.org")
                    inputMethodHints: Qt.ImhEmailCharactersOnly
                }

                QQC2.Button {
                    icon.name: "list-add"
                    implicitWidth: implicitHeight
                    enabled: isNotEmptyStr(toAddEmail.text)
                    QQC2.ToolTip {
                        text: i18n("Add email alias")
                    }
                    onClicked: {
                        emailAliasesRepeater.emailAliases.push(toAddEmail.text);
                        identity.emailAliases = emailAliasesRepeater.emailAliases;
                        toAddEmail.clear();
                    }
                }
            }
        }
    }
}
