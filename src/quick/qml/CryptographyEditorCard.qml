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
            id: pgpSigningDelegate
            text: i18n("OpenPGP signing key")
            model: cryptographyEditorBackend.openPgpKeyListModel
            textRole: "display"
            valueRole: "keyByteArray"
            onCurrentValueChanged: root.identity.pgpSigningKey = currentValue
        }

        MobileForm.FormComboBoxDelegate {
            id: identityDelegate
            text: i18n("OpenPGP encryption key")
            model: cryptographyEditorBackend.openPgpKeyListModel
            textRole: "display"
            valueRole: "keyByteArray"
            onCurrentValueChanged: root.identity.pgpEncryptionKey = currentValue
        }

        MobileForm.FormComboBoxDelegate {
            id: smimeSigningDelegate
            text: i18n("S/MIME signing key")
            model: cryptographyEditorBackend.smimeKeyListModel
            textRole: "display"
            valueRole: "keyByteArray"
            onCurrentValueChanged: root.identity.smimeSigningKey = currentValue
        }

        MobileForm.FormComboBoxDelegate {
            id: smimeEncryptionDelegate
            text: i18n("S/MIME encryption key")
            model: cryptographyEditorBackend.smimeKeyListModel
            textRole: "display"
            valueRole: "keyByteArray"
            onCurrentValueChanged: root.identity.smimeEncryptionKey = currentValue
        }
    }
}
