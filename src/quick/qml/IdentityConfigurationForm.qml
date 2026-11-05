// SPDX-FileCopyrightText: 2023 Claudio Cambra <claudio.cambra@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kidentitymanagement

FormCard.FormCard {
    id: root

    readonly property IdentityModel _identityModel: IdentityModel {}
    property var cryptographyEditorBackend: CryptographyEditorBackend {}

    autoSeparators: true

    Repeater {
        id: identityRepeater

        model: root._identityModel

        delegate: FormCard.FormButtonDelegate {
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

    FormCard.FormButtonDelegate {
        text: i18nc("@title", "Add Identity")
        icon.name: "list-add-symbolic"
        onClicked: {
            pageStack.pushDialogLayer(Qt.resolvedUrl("IdentityEditorPage.qml"), {
                mode: IdentityEditorBackend.CreateMode,
                allowDelete: identityRepeater.count > 1,
                identityName: i18nc("@title", "Add Identity"),
                cryptographyEditorBackend: root.cryptographyEditorBackend
            }, {title: i18nc("@title", "Add Identity")});
        }
    }
}
