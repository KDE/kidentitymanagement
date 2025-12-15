// SPDX-FileCopyrightText: 2023 Claudio Cambra <claudio.cambra@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import Qt.labs.platform

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard 1.0 as FormCard
import org.kde.kidentitymanagement 1.0

ColumnLayout {
    id: root

    required property IdentityEditorBackend backend
    readonly property bool isValid: identityDelegate.isValid
        && nameDelegate.isValid
        && emailDelegate.isValid

    function isNotEmptyStr(str) {
        return str.trim().length > 0;
    }

    spacing: 0

    FormCard.FormHeader {
        title: i18n("Identity")
    }

    FormCard.FormCard {
        FormCard.FormTextFieldDelegate {
            id: identityDelegate

            readonly property bool isValid: text.trim().length > 0

            label: i18nc("@label:textbox", "Identity Name")
            text: root.backend.identity.identityName
            onTextChanged: root.backend.identity.identityName = text.trim()
        }
        FormCard.FormTextFieldDelegate {
            id: nameDelegate

            readonly property bool isValid: text.trim().length > 0

            label: i18n("Your name")
            text: root.backend.identity.fullName
            onTextChanged: root.backend.identity.fullName = text.trim()
        }

        FormCard.FormTextFieldDelegate {
            id: emailDelegate

            readonly property bool isValid: text.trim().length > 0

            label: i18n("Email address")
            text: root.backend.identity.primaryEmailAddress
            onTextChanged: root.backend.identity.primaryEmailAddress = text.trim()
            inputMethodHints: Qt.ImhEmailCharactersOnly
        }
    }

    FormCard.FormHeader {
        title: i18n("E-mail aliases")
    }

    FormCard.FormCard {
        Repeater {
            id: emailAliasesRepeater

            property list<string> emailAliases: root.backend.identity.emailAliases

            model: emailAliases

            delegate: FormCard.AbstractFormDelegate {
                id: emailRow

                background: null

                contentItem: RowLayout {
                    QQC2.TextField {
                        id: emailAliasField
                        Layout.fillWidth: true
                        text: modelData
                        inputMethodHints: Qt.ImhEmailCharactersOnly
                        onEditingFinished: {
                            let emailAliases = emailAliasesRepeater.emailAliases;
                            emailAliases[model.index] = text;
                            root.backend.identity.emailAliases = emailAliases;
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
                            root.backend.identity.emailAliases = emailAliases;
                            emailAliasesRepeater.emailAliases = emailAliases;
                        }
                    }
                }
            }
        }

        FormCard.AbstractFormDelegate {
            background: null

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
                        root.backend.identity.emailAliases = emailAliases;
                        emailAliasesRepeater.emailAliases = emailAliases;
                        toAddEmail.clear();
                    }
                }
            }
        }
    }
}
