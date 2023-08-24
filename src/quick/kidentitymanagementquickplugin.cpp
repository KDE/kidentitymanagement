// SPDX-FileCopyrightText: 2023 Claudio Cambra <claudio.cambra@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "kidentitymanagementquickplugin.h"

#include <QQmlEngine>

#include "identityutils.h"

void KIdentityManagementQuickPlugin::registerTypes(const char *uri)
{
    Q_UNUSED(uri);

    qmlRegisterSingletonType<KIdentityManagement::Quick::IdentityUtils>(uri, 1, 0, "IdentityUtils", [](QQmlEngine *engine, QJSEngine *scriptEngine) {
        Q_UNUSED(engine)
        Q_UNUSED(scriptEngine)
        return new KIdentityManagement::Quick::IdentityUtils;
    });
}