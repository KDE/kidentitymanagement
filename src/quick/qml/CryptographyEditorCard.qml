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

    required property var identity
    required property var cryptographyEditorBackend

    Layout.fillWidth: true
    Layout.topMargin: Kirigami.Units.largeSpacing

    contentItem: ColumnLayout {
        spacing: 0

        MobileForm.FormCardHeader {
            title: i18n("Cryptography")
        }

        MobileForm.FormComboBoxDelegate {
            id: pgpSigningOrCombinedDelegate

            readonly property bool combinedMode: combinedPgpModeCheckBox.checked

            text: combinedMode ? i18n("OpenPGP key") : i18n("OpenPGP signing key")
            model: cryptographyEditorBackend.openPgpKeyListModel
            textRole: "display"
            valueRole: "keyByteArray"
            onActivated: {
                root.identity.pgpSigningKey = currentValue;

                if (combinedMode) {
                    root.identity.pgpEncryptionKey = currentValue;
                }
            }
        }

        MobileForm.FormCheckDelegate {
            id: combinedPgpModeCheckBox
            text: i18n("Use same OpenPGP key for encryption and signing")
            checked: pgpSigningOrCombinedDelegate.currentValue === pgpEncryptionDelegate.currentValue
        }

        MobileForm.FormComboBoxDelegate {
            id: pgpEncryptionDelegate
            text: i18n("OpenPGP encryption key")
            model: cryptographyEditorBackend.openPgpKeyListModel
            textRole: "display"
            valueRole: "keyByteArray"
            onActivated: root.identity.pgpEncryptionKey = currentValue
            visible: !combinedPgpModeCheckBox.checked
        }

        MobileForm.FormComboBoxDelegate {
            id: smimeSigningOrCombinedDelegate

            property bool combinedMode: combinedSmimeModeCheckBox.checked

            text: i18n("S/MIME signing key")
            model: cryptographyEditorBackend.smimeKeyListModel
            textRole: "display"
            valueRole: "keyByteArray"
            onActivated: {
                root.identity.smimeSigningKey = currentValue;

                if (combinedMode) {
                    root.identity.smimeEncryptionKey = currentValue;
                }
            }
        }

        MobileForm.FormCheckDelegate {
            id: combinedSmimeModeCheckBox
            text: i18n("Use same S/MIME key for encryption and signing")
            checked: smimeSigningOrCombinedDelegate.currentValue === smimeEncryptionDelegate.currentValue
        }

        MobileForm.FormComboBoxDelegate {
            id: smimeEncryptionDelegate
            text: i18n("S/MIME encryption key")
            model: cryptographyEditorBackend.smimeKeyListModel
            textRole: "display"
            valueRole: "keyByteArray"
            onActivated: root.identity.smimeEncryptionKey = currentValue
            visible: !combinedSmimeModeCheckBox.checked
        }
    }
}
