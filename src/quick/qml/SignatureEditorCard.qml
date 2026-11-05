// SPDX-FileCopyrightText: 2026 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

pragma ComponentBehavior: Bound

import QtQuick
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kidentitymanagement

FormCard.FormCard {
    id: root

    required property IdentityEditorBackend backend

    autoSeparators: true

    FormCard.FormSwitchDelegate {
        id: signatureEnabled
        text: i18ndc("libkpimidentities6", "@option:check", "Enable signature")
        checked: root.backend.signatureEnabled
        onToggled: root.backend.signatureEnabled = signatureEnabled.checked
    }

    FormCard.FormTextAreaDelegate {
        id: signatureText
        label: i18ndc("libkpimidentities6", "@label:textbox", "Signature text")
        enabled: root.backend.signatureEnabled
        text: root.backend.signatureText
        onTextChanged: if (signatureText.fieldActiveFocus) {
            root.backend.signatureText = signatureText.text;
        }
    }
}
