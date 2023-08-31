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

using namespace KIdentityManagementQuick;

void KIdentityManagementQuickPlugin::registerTypes(const char *uri)
{
    // @uri org.kde.kidentitymanagement
    Q_ASSERT(uri == QByteArray("org.kde.kidentitymanagement"));

    qmlRegisterSingletonType<IdentityUtils>(uri, 1, 0, "IdentityUtils", [](QQmlEngine *engine, QJSEngine *scriptEngine) {
        Q_UNUSED(engine)
        Q_UNUSED(scriptEngine)
        return new IdentityUtils;
    });

    qmlRegisterType<CryptographyEditorBackend>(uri, 1, 0, "CryptographyEditorBackend");
    qmlRegisterType<IdentityEditorBackend>(uri, 1, 0, "IdentityEditorBackend");
    qmlRegisterType<KIdentityManagementCore::IdentityModel>(uri, 1, 0, "IdentityModel");

    qRegisterMetaType<CryptographyBackendInterfacePtr>("CryptographyBackendInterfacePtr");
    qRegisterMetaType<KIdentityManagementCore::Identity>("Identity");
    qRegisterMetaType<KeyUseTypes::KeyUse>("KeyUseTypes::KeyUse");

    qmlRegisterUncreatableType<KeyUseTypes>(uri, 1, 0, "KeyUseTypes", QStringLiteral("Cannot instantiate KeyUseTypes wrapper!"));
}
