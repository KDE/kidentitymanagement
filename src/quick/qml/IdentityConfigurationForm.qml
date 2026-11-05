// SPDX-FileCopyrightText: 2023 Claudio Cambra <claudio.cambra@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kirigamiaddons.components as Components
import org.kde.kidentitymanagement

FormCard.FormCard {
    id: root

    readonly property IdentityModel _identityModel: IdentityModel {}
    property var cryptographyEditorBackend: CryptographyEditorBackend {}

    /** Callback taking an identity's email address and returning its avatar image URL. */
    property var imageSourceForEmail: function(emailAddress: string): url {
        return "";
    }

    autoSeparators: true

    Repeater {
        id: identityRepeater

        model: root._identityModel

        delegate: FormCard.FormButtonDelegate {
            id: identityDelegate

            required property var model

            text: identityDelegate.model.identityName
            description: identityDelegate.model.email
            leading: Components.Avatar {
                implicitWidth: Kirigami.Units.iconSizes.medium
                implicitHeight: Kirigami.Units.iconSizes.medium
                name: identityDelegate.model.identityName
                source: root.imageSourceForEmail(identityDelegate.model.email)
            }
            onClicked: {
                (root.Kirigami.PageStack.pageStack as Kirigami.PageRow).pushDialogLayer(Qt.resolvedUrl("IdentityEditorPage.qml"), {
                    mode: IdentityEditorBackend.EditMode,
                    identityUoid: identityDelegate.model.uoid,
                    allowDelete: identityRepeater.count > 1,
                    identityName: identityDelegate.model.display,
                    cryptographyEditorBackend: root.cryptographyEditorBackend
                }, {title: i18nc("@title", "Edit Identity")});
            }
        }
    }

    FormCard.FormButtonDelegate {
        text: i18nc("@title", "Add Identity")
        icon.name: "list-add-symbolic"
        onClicked: {
            (root.Kirigami.PageStack.pageStack as Kirigami.PageRow).pushDialogLayer(Qt.resolvedUrl("IdentityEditorPage.qml"), {
                mode: IdentityEditorBackend.CreateMode,
                allowDelete: identityRepeater.count > 1,
                identityName: i18nc("@title", "Add Identity"),
                cryptographyEditorBackend: root.cryptographyEditorBackend
            }, {title: i18nc("@title", "Add Identity")});
        }
    }
}
