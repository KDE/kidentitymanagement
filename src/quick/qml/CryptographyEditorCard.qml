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

    required property var identity
    onIdentityChanged: _updateComboIndices()
    required property var cryptographyEditorBackend

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
            root.identity.pgpSigningKey = currentValue;

            if (combinedMode) {
                root.identity.pgpEncryptionKey = currentValue;
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
            const pgpEncryptionKey = root.cryptographyEditorBackend.stringFromKeyByteArray(root.identity.pgpEncryptionKey);
            const pgpSigningKey = root.cryptographyEditorBackend.stringFromKeyByteArray(root.identity.pgpSigningKey);
            checked = pgpEncryptionKey === pgpSigningKey;
        }

        text: i18ndc("libkpimidentities6", "@label", "Use same OpenPGP key for encryption and signing")
        onClicked: {
            if (!checked) {
                return;
            }

            const pgpEncryptionKey = root.cryptographyEditorBackend.stringFromKeyByteArray(root.identity.pgpEncryptionKey);
            const pgpSigningKey = root.cryptographyEditorBackend.stringFromKeyByteArray(root.identity.pgpSigningKey);

            // Use the signing key as this is represented by the top combo box
            if (pgpEncryptionKey !== pgpSigningKey) {
                root.identity.pgpEncryptionKey = root.identity.pgpSigningKey;
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
        onActivated: root.identity.pgpEncryptionKey = currentValue
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
            root.identity.smimeSigningKey = currentValue;

            if (combinedMode) {
                root.identity.smimeEncryptionKey = currentValue;
                smimeEncryptionDelegate.updateIndex();
            }
        }
    }

    FormCard.FormDelegateSeparator { above: combinedSmimeModeCheckBox; below: smimeSigningOrCombinedDelegate }

    FormCard.FormCheckDelegate {
        id: combinedSmimeModeCheckBox

        function updateChecked() {
            const smimeEncryptionKey = root.cryptographyEditorBackend.stringFromKeyByteArray(root.identity.smimeEncryptionKey);
            const smimeSigningKey = root.cryptographyEditorBackend.stringFromKeyByteArray(root.identity.smimeSigningKey);
            checked = smimeEncryptionKey === smimeSigningKey;
        }

        text: i18ndc("libkpimidentities6", "@label", "Use same S/MIME key for encryption and signing")
        onClicked: {
            if (!checked) {
                return;
            }

            const smimeEncryptionKey = root.cryptographyEditorBackend.stringFromKeyByteArray(root.identity.smimeEncryptionKey);
            const smimeSigningKey = root.cryptographyEditorBackend.stringFromKeyByteArray(root.identity.smimeSigningKey);

            // Use the signing key as this is represented by the top combo box
            if (smimeEncryptionKey !== smimeSigningKey) {
                root.identity.smimeEncryptionKey = root.identity.smimeSigningKey;
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
        onActivated: root.identity.smimeEncryptionKey = currentValue
        visible: !combinedSmimeModeCheckBox.checked
    }
}
