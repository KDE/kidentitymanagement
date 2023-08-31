// SPDX-FileCopyrightText: 2023 Claudio Cambra <claudio.cambra@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "kidentitymanagementquickplugin.h"

#include <QQmlEngine>

#include "identity.h"
#include "identityeditorbackend.h"
#include "identitymodel.h"
#include "identityutils.h"
#include "keylistmodelinterface.h"
#include <KIdentityManagementQuick/CryptographyEditorBackend>

using namespace KIdentityManagement;

void KIdentityManagementQuickPlugin::registerTypes(const char *uri)
{
    // @uri org.kde.kidentitymanagement
    Q_ASSERT(uri == QByteArray("org.kde.kidentitymanagement"));

    qmlRegisterSingletonType<Quick::IdentityUtils>(uri, 1, 0, "IdentityUtils", [](QQmlEngine *engine, QJSEngine *scriptEngine) {
        Q_UNUSED(engine)
        Q_UNUSED(scriptEngine)
        return new Quick::IdentityUtils;
    });

    qmlRegisterType<Quick::CryptographyEditorBackend>(uri, 1, 0, "CryptographyEditorBackend");
    qmlRegisterType<Quick::IdentityEditorBackend>(uri, 1, 0, "IdentityEditorBackend");
    qmlRegisterType<IdentityModel>(uri, 1, 0, "IdentityModel");

    qRegisterMetaType<Quick::CryptographyBackendInterfacePtr>("CryptographyBackendInterfacePtr");
    qRegisterMetaType<Identity>("Identity");
    qRegisterMetaType<Quick::KeyUseTypes::KeyUse>("KeyUseTypes::KeyUse");

    qmlRegisterUncreatableType<Quick::KeyUseTypes>(uri, 1, 0, "KeyUseTypes", QStringLiteral("Cannot instantiate KeyUseTypes wrapper!"));
}
