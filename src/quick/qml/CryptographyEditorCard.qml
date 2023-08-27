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
    required property CryptographyEditorBackend cryptographyEditorBackend

    Layout.fillWidth: true
    Layout.topMargin: Kirigami.Units.largeSpacing

    contentItem: ColumnLayout {
        spacing: 0

        MobileForm.FormCardHeader {
            title: i18n("Cryptography")
        }
    }
}
