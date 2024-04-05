// SPDX-FileCopyrightText: 2023 Claudio Cambra <claudio.cambra@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import Qt.labs.platform

import org.kde.kirigami 2.19 as Kirigami
import org.kde.kirigamiaddons.formcard 1.0 as FormCard
import org.kde.kidentitymanagement 1.0

ColumnLayout {
    id: root

    required property var identity
    property alias toAddEmailText: toAddEmail.text

    function isNotEmptyStr(str) {
        return str.trim().length > 0;
    }

    spacing: 0

    FormCard.FormHeader {
        title: i18n("Identity")
    }

    FormCard.FormCard {
        FormCard.FormTextFieldDelegate {
            id: nameDelegate
            Layout.fillWidth: true
            label: i18n("Your name")
            text: root.identity.fullName
            onTextChanged: root.identity.fullName = text
        }

        FormCard.FormTextFieldDelegate {
            id: emailDelegate
            label: i18n("Email address")
            text: root.identity.primaryEmailAddress
            onTextChanged: root.identity.primaryEmailAddress = text
            inputMethodHints: Qt.ImhEmailCharactersOnly
        }
    }

    FormCard.FormHeader {
        title: i18n("E-mail aliases")
    }

    FormCard.FormCard {
        Repeater {
            id: emailAliasesRepeater

            property var emailAliases: root.identity.emailAliases

            model: emailAliases

            delegate: FormCard.AbstractFormDelegate {
                id: emailRow

                Layout.fillWidth: true

                contentItem: RowLayout {
                    QQC2.TextField {
                        id: emailAliasField
                        Layout.fillWidth: true
                        text: modelData
                        inputMethodHints: Qt.ImhEmailCharactersOnly
                        onEditingFinished: {
                            let emailAliases = emailAliasesRepeater.emailAliases;
                            emailAliases[model.index] = text;
                            identity.emailAliases = emailAliases;
                            emailAliasesRepeater.emailAliases = emailAliases;
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
                            emailAliasesRepeater.emailAliases = emailAliases;
                        }
                    }
                }
            }
        }

        FormCard.AbstractFormDelegate {
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
                        let emailAliases = emailAliasesRepeater.emailAliases;
                        emailAliases.push(toAddEmail.text);
                        identity.emailAliases = emailAliasesRepeater.emailAliases;
                        emailAliasesRepeater.emailAliases = emailAliases;
                        toAddEmail.clear();
                    }
                }
            }
        }
    }
}
