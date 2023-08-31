// SPDX-FileCopyrightText: 2023 Claudio Cambra <claudio.cambra@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2

import org.kde.kirigami 2.20 as Kirigami
import org.kde.kidentitymanagement 1.0
import org.kde.kirigamiaddons.formcard 1.0 as FormCard

FormCard.FormCardPage {
    id: root

    property alias mode: backend.mode
    property alias identityUoid: backend.identityUoid

    required property bool allowDelete
    required property string identityName
    required property var cryptographyEditorBackend
    onCryptographyEditorBackendChanged: cryptographyEditorBackend.identity = identity

    readonly property IdentityEditorBackend backend: IdentityEditorBackend {
        id: backend
        mode: IdentityEditorBackend.CreateMode
    }
    readonly property var identity: backend.identity
    onIdentityChanged: cryptographyEditorBackend.identity = identity

    readonly property QQC2.Action submitAction: QQC2.Action {
        id: submitAction
        enabled: !root.identity.isNull
        shortcut: "Return"
        onTriggered: {
            root.backend.saveIdentity(root.identity);
            root.closeDialog();
        }
    }

    title: if (mode === IdentityEditorBackend.CreateMode) {
        return i18n("Add Identity");
    } else {
        return i18n("Edit Identity");
    }

    header: QQC2.Control {
        id: errorContainer

        property bool displayError: false
        property string errorMessage: ''

        padding: contentItem.visible ? Kirigami.Units.smallSpacing : 0
        leftPadding: padding
        rightPadding: padding
        topPadding: padding
        bottomPadding: padding
        contentItem: Kirigami.InlineMessage {
            type: Kirigami.MessageType.Error
            visible: errorContainer.displayError
            text: errorContainer.errorMessage
            showCloseButton: true
        }
    }

    BasicIdentityEditorCard {
        identity: root.identity
    }

    FormCard.FormHeader {
        visible: root.cryptographyEditorBackend.validBackend
        title: i18ndc("libkpimidentities5", "@title:group", "Cryptography")
    }

    Loader {
        Layout.fillWidth: true
        active: root.cryptographyEditorBackend.validBackend
        sourceComponent: CryptographyEditorCard {
            identity: root.identity
            cryptographyEditorBackend: root.cryptographyEditorBackend
        }
    }

    footer: ColumnLayout {
        spacing: 0

        Kirigami.Separator {
            Layout.fillWidth: true
        }

        QQC2.DialogButtonBox {
            Layout.fillWidth: true

            standardButtons: QQC2.DialogButtonBox.Cancel

            QQC2.Button {
                icon.name: "delete"
                text: i18n("Delete")
                visible: root.mode === IdentityEditorBackend.EditMode
                enabled: root.allowDelete
                QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.DestructiveRole
            }

            QQC2.Button {
                icon.name: root.mode === IdentityEditorBackend.EditMode ? "document-save" : "list-add"
                text: root.mode === IdentityEditorBackend.EditMode ? i18n("Save") : i18n("Add")
                QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.AcceptRole
            }

            onRejected: root.closeDialog()
            onAccepted: submitAction.trigger();
            onDiscarded: {
                dialogLoader.active = true;
                dialogLoader.item.open();
            }

            Loader {
                id: dialogLoader
                sourceComponent: Kirigami.PromptDialog {
                    id: dialog
                    title: i18n("Delete %1", root.identityName)
                    subtitle: i18n("Are you sure you want to delete this identity?")
                    standardButtons: Kirigami.Dialog.NoButton

                    customFooterActions: [
                        Kirigami.Action {
                            text: i18n("Delete")
                            iconName: "delete"
                            onTriggered: {
                                IdentityUtils.removeIdentity(root.identityName);
                                dialog.close();
                            }
                        },
                        Kirigami.Action {
                            text: i18n("Cancel")
                            iconName: "dialog-cancel"
                            onTriggered: dialog.close()
                        }
                    ]
                }
            }
        }
    }
}
