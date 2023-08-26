// SPDX-FileCopyrightText: 2023 Claudio Cambra <claudio.cambra@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QObject>

class IdentityModelTester : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
};
