// SPDX-FileCopyrightText: 2023 Claudio Cambra <claudio.cambra@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "kidentitymanagementquickplugin.h"

#include <QQmlEngine>

#include "identitymodel.h"
#include "identityutils.h"

using namespace KIdentityManagement;

void KIdentityManagementQuickPlugin::registerTypes(const char *uri)
{
    qmlRegisterSingletonType<Quick::IdentityUtils>(uri, 1, 0, "IdentityUtils", [](QQmlEngine *engine, QJSEngine *scriptEngine) {
        Q_UNUSED(engine)
        Q_UNUSED(scriptEngine)
        return new Quick::IdentityUtils;
    });

    qmlRegisterType<Quick::IdentityModel>(uri, 1, 0, "IdentityModel");
}