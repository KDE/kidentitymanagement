// SPDX-FileCopyrightText: 2023 Claudio Cambra <claudio.cambra@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.19 as Kirigami
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm
import org.kde.kidentitymanagement 1.0

MobileForm.FormCard {
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

    Layout.fillWidth: true
    Layout.topMargin: Kirigami.Units.largeSpacing

    contentItem: ColumnLayout {
        spacing: 0

        MobileForm.FormCardHeader {
            title: i18n("Cryptography")
        }

        MobileForm.FormComboBoxDelegate {
            id: pgpSigningOrCombinedDelegate

            function updateIndex() {
                currentIndex = root._index(cryptographyEditorBackend.openPgpKeyListModel, KeyUseTypes.KeySigningUse);
            }

            readonly property bool combinedMode: combinedPgpModeCheckBox.checked

            text: combinedMode ? i18n("OpenPGP key") : i18n("OpenPGP signing key")
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

        MobileForm.FormCheckDelegate {
            id: combinedPgpModeCheckBox

            function updateChecked() {
                const pgpEncryptionKey = root.cryptographyEditorBackend.stringFromKeyByteArray(root.identity.pgpEncryptionKey);
                const pgpSigningKey = root.cryptographyEditorBackend.stringFromKeyByteArray(root.identity.pgpSigningKey);
                checked = pgpEncryptionKey === pgpSigningKey;
            }

            text: i18n("Use same OpenPGP key for encryption and signing")
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

        MobileForm.FormComboBoxDelegate {
            id: pgpEncryptionDelegate

            function updateIndex() {
                currentIndex = root._index(cryptographyEditorBackend.openPgpKeyListModel, KeyUseTypes.KeyEncryptionUse);
            }

            text: i18n("OpenPGP encryption key")
            model: cryptographyEditorBackend.openPgpKeyListModel
            textRole: "display"
            valueRole: "keyByteArray"
            onActivated: root.identity.pgpEncryptionKey = currentValue
            visible: !combinedPgpModeCheckBox.checked
        }

        MobileForm.FormComboBoxDelegate {
            id: smimeSigningOrCombinedDelegate

            function updateIndex() {
                currentIndex = root._index(cryptographyEditorBackend.smimeKeyListModel, KeyUseTypes.KeySigningUse);
            }

            property bool combinedMode: combinedSmimeModeCheckBox.checked

            text: i18n("S/MIME signing key")
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

        MobileForm.FormCheckDelegate {
            id: combinedSmimeModeCheckBox

            function updateChecked() {
                const smimeEncryptionKey = root.cryptographyEditorBackend.stringFromKeyByteArray(root.identity.smimeEncryptionKey);
                const smimeSigningKey = root.cryptographyEditorBackend.stringFromKeyByteArray(root.identity.smimeSigningKey);
                checked = smimeEncryptionKey === smimeSigningKey;
            }

            text: i18n("Use same S/MIME key for encryption and signing")
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

        MobileForm.FormComboBoxDelegate {
            id: smimeEncryptionDelegate

            function updateIndex() {
                currentIndex = root._index(cryptographyEditorBackend.smimeKeyListModel, KeyUseTypes.KeyEncryptionUse);
            }

            text: i18n("S/MIME encryption key")
            model: cryptographyEditorBackend.smimeKeyListModel
            textRole: "display"
            valueRole: "keyByteArray"
            onActivated: root.identity.smimeEncryptionKey = currentValue
            visible: !combinedSmimeModeCheckBox.checked
        }
    }
}
