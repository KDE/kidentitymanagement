// SPDX-FileCopyrightText: 2023 Claudio Cambra <claudio.cambra@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2

import org.kde.kirigami 2.20 as Kirigami
import org.kde.kirigamiaddons.formcard 1.0 as FormCard
import org.kde.kidentitymanagement 1.0

FormCard.FormCard {
    id: root

    readonly property IdentityModel _identityModel: IdentityModel {}
    property var cryptographyEditorBackend: CryptographyEditorBackend {}

    Repeater {
        id: identityRepeater

        model: root._identityModel

        delegate: FormCard.FormButtonDelegate {
            leadingPadding: Kirigami.Units.largeSpacing
            text: model.display
            onClicked: {
                pageStack.pushDialogLayer(Qt.resolvedUrl("IdentityEditorPage.qml"), {
                    mode: IdentityEditorBackend.EditMode,
                    identityUoid: model.uoid,
                    allowDelete: identityRepeater.count > 1,
                    identityName: model.display,
                    cryptographyEditorBackend: root.cryptographyEditorBackend
                }, {title: i18nc("@title", "Edit Identity")});
            }
        }
    }
}
