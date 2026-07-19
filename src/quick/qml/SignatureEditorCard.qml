// SPDX-FileCopyrightText: 2026 Florian Richer <florian.richer@protonmail.com>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard 1.0 as FormCard
import org.kde.kidentitymanagement 1.0


FormCard.FormCard {
    id: root

    required property IdentityEditorBackend backend
    required property SignatureEditorBackend signatureEditorBackend

    FormCard.FormSwitchDelegate {
        text: i18ndc("libkpimidentities6", "@label", "Enable signature")
        onCheckedChanged: signatureEditorBackend.enabled = checked
    }

    FormCard.FormComboBoxDelegate {
        id: signatureTypeDelegate

        function updateIndex(): void {
            currentIndex = model.map(entry => entry.value).indexOf(root.signatureEditorBackend.type);
        }

        visible: signatureEditorBackend.enabled
        text: i18ndc("libkpimidentities6", "@label", "Obtain signature text from")
        model: [
            { text: i18ndc("libkpimidentities6", "@item:inlistbox", "Input Field Below"), value: SignatureEditorBackend.Inlined },
            { text: i18ndc("libkpimidentities6", "@item:inlistbox", "File"), value: SignatureEditorBackend.FromFile },
            { text: i18ndc("libkpimidentities6", "@item:inlistbox", "Output of Command"), value: SignatureEditorBackend.FromCommand },
        ]
        textRole: "text"
        valueRole: "value"
        Component.onCompleted: updateIndex()
        onActivated: root.signatureEditorBackend.type = currentValue
    }

    FormCard.FormSwitchDelegate {
        visible: signatureEditorBackend.enabled && signatureEditorBackend.type === SignatureEditorBackend.Inlined
        text: i18ndc("libkpimidentities6", "@label", "HTML Format")
        onCheckedChanged: signatureEditorBackend.htmlFormat = checked
    }

    FormCard.FormTextAreaDelegate {
        visible: signatureEditorBackend.enabled && signatureEditorBackend.type === SignatureEditorBackend.Inlined
        label: i18ndc("libkpimidentities6", "@label", "Signature Text:")
        text: signatureEditorBackend.signatureText
        onTextChanged: signatureEditorBackend.signatureText = text
    }

    FormCard.FormFileDelegate {
        visible: signatureEditorBackend.enabled && signatureEditorBackend.type === SignatureEditorBackend.FromFile
        label: i18ndc("libkpimidentities6", "@label", "Specify file:")
        selectedFile: signatureEditorBackend.file
        onAccepted: signatureEditorBackend.file = selectedFile
    }

    FormCard.FormTextFieldDelegate {
        visible: signatureEditorBackend.enabled && signatureEditorBackend.type === SignatureEditorBackend.FromCommand
        label: i18ndc("libkpimidentities6", "@label", "Specify command:")
        text: signatureEditorBackend.command
        onTextChanged: signatureEditorBackend.command = text
    }
}
