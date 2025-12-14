// SPDX-FileCopyrightText: 2023 Claudio Cambra <claudio.cambra@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard 1.0 as FormCard
import org.kde.kidentitymanagement 1.0

FormCard.FormCard {
    id: root

    // Since Identity is not a QObject and its properties do not emit signals,
    // we use these convenience functions to update the values of each interactive
    // component of this card when editing happens

    function _index(model, keyUse) {
        return cryptographyEditorBackend.indexForIdentity(model, identity, keyUse).row
    }

    function _updateComboIndices() {
        combinedPgpModeCheckBox.updateChecked();
        combinedSmimeModeCheckBox.updateChecked();

        pgpSigningOrCombinedDelegate.updateIndex();
        pgpEncryptionDelegate.updateIndex();
        smimeSigningOrCombinedDelegate.updateIndex();
        smimeEncryptionDelegate.updateIndex();
    }

    required property IdentityEditorBackend backend
    required property var cryptographyEditorBackend

    Connections {
        target: backend

        function onIdentityChanged(): void {
            _updateComboIndices()
        }
    }

    Component.onCompleted: _updateComboIndices()

    FormCard.FormComboBoxDelegate {
        id: pgpSigningOrCombinedDelegate

        function updateIndex() {
            currentIndex = root._index(cryptographyEditorBackend.openPgpKeyListModel, KeyUseTypes.KeySigningUse);
        }

        readonly property bool combinedMode: combinedPgpModeCheckBox.checked

        text: combinedMode ? i18ndc("libkpimidentities6", "@label", "OpenPGP key") : i18ndc("libkpimidentities6", "@label", "OpenPGP signing key")
        model: cryptographyEditorBackend.openPgpKeyListModel
        textRole: "display"
        valueRole: "keyByteArray"
        onActivated: {
            root.backend.identity.pgpSigningKey = currentValue;

            if (combinedMode) {
                root.backend.identity.pgpEncryptionKey = currentValue;
                pgpEncryptionDelegate.updateIndex();
            }
        }
    }

    FormCard.FormDelegateSeparator {
        above: combinedPgpModeCheckBox; below: pgpSigningOrCombinedDelegate
    }

    FormCard.FormCheckDelegate {
        id: combinedPgpModeCheckBox

        function updateChecked() {
            const pgpEncryptionKey = root.cryptographyEditorBackend.stringFromKeyByteArray(root.backend.identity.pgpEncryptionKey);
            const pgpSigningKey = root.cryptographyEditorBackend.stringFromKeyByteArray(root.backend.identity.pgpSigningKey);
            checked = pgpEncryptionKey === pgpSigningKey;
        }

        text: i18ndc("libkpimidentities6", "@label", "Use same OpenPGP key for encryption and signing")
        onClicked: {
            if (!checked) {
                return;
            }

            const pgpEncryptionKey = root.cryptographyEditorBackend.stringFromKeyByteArray(root.backend.identity.pgpEncryptionKey);
            const pgpSigningKey = root.cryptographyEditorBackend.stringFromKeyByteArray(root.backend.identity.pgpSigningKey);

            // Use the signing key as this is represented by the top combo box
            if (pgpEncryptionKey !== pgpSigningKey) {
                root.backend.identity.pgpEncryptionKey = root.backend.identity.pgpSigningKey;
            }
        }
    }

    FormCard.FormDelegateSeparator {
        below: combinedPgpModeCheckBox
        above: pgpEncryptionDelegate
        visible: pgpEncryptionDelegate.visible
    }

    FormCard.FormComboBoxDelegate {
        id: pgpEncryptionDelegate

        function updateIndex() {
            currentIndex = root._index(cryptographyEditorBackend.openPgpKeyListModel, KeyUseTypes.KeyEncryptionUse);
        }

        text: i18ndc("libkpimidentities6", "@label", "OpenPGP encryption key")
        model: cryptographyEditorBackend.openPgpKeyListModel
        textRole: "display"
        valueRole: "keyByteArray"
        onActivated: root.backend.identity.pgpEncryptionKey = currentValue
        visible: !combinedPgpModeCheckBox.checked
    }

    FormCard.FormDelegateSeparator {
        above: smimeSigningOrCombinedDelegate
        below: pgpEncryptionDelegate.visible ? pgpEncryptionDelegate : combinedPgpModeCheckBox
    }

    FormCard.FormComboBoxDelegate {
        id: smimeSigningOrCombinedDelegate

        function updateIndex() {
            currentIndex = root._index(cryptographyEditorBackend.smimeKeyListModel, KeyUseTypes.KeySigningUse);
        }

        property bool combinedMode: combinedSmimeModeCheckBox.checked

        text: i18ndc("libkpimidentities6", "@label", "S/MIME signing key")
        model: cryptographyEditorBackend.smimeKeyListModel
        textRole: "display"
        valueRole: "keyByteArray"
        onActivated: {
            root.backend.identity.smimeSigningKey = currentValue;

            if (combinedMode) {
                root.backend.identity.smimeEncryptionKey = currentValue;
                smimeEncryptionDelegate.updateIndex();
            }
        }
    }

    FormCard.FormDelegateSeparator { above: combinedSmimeModeCheckBox; below: smimeSigningOrCombinedDelegate }

    FormCard.FormCheckDelegate {
        id: combinedSmimeModeCheckBox

        function updateChecked() {
            const smimeEncryptionKey = root.cryptographyEditorBackend.stringFromKeyByteArray(root.backend.identity.smimeEncryptionKey);
            const smimeSigningKey = root.cryptographyEditorBackend.stringFromKeyByteArray(root.backend.identity.smimeSigningKey);
            checked = smimeEncryptionKey === smimeSigningKey;
        }

        text: i18ndc("libkpimidentities6", "@label", "Use same S/MIME key for encryption and signing")
        onClicked: {
            if (!checked) {
                return;
            }

            const smimeEncryptionKey = root.cryptographyEditorBackend.stringFromKeyByteArray(root.backend.identity.smimeEncryptionKey);
            const smimeSigningKey = root.cryptographyEditorBackend.stringFromKeyByteArray(root.backend.identity.smimeSigningKey);

            // Use the signing key as this is represented by the top combo box
            if (smimeEncryptionKey !== smimeSigningKey) {
                root.backend.identity.smimeEncryptionKey = root.backend.identity.smimeSigningKey;
            }
        }
    }

    FormCard.FormDelegateSeparator {
        above: smimeEncryptionDelegate
        below: combinedSmimeModeCheckBox
        visible: !combinedSmimeModeCheckBox.checked
    }

    FormCard.FormComboBoxDelegate {
        id: smimeEncryptionDelegate

        function updateIndex() {
            currentIndex = root._index(cryptographyEditorBackend.smimeKeyListModel, KeyUseTypes.KeyEncryptionUse);
        }

        text: i18ndc("libkpimidentities6", "@label", "S/MIME encryption key")
        model: cryptographyEditorBackend.smimeKeyListModel
        textRole: "display"
        valueRole: "keyByteArray"
        onActivated: root.backend.identity.smimeEncryptionKey = currentValue
        visible: !combinedSmimeModeCheckBox.checked
    }
}
