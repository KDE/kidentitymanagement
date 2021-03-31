/*
    SPDX-FileCopyrightText: 2008 Thomas McGuire <mcguire@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QObject>

namespace KIdentityManagement
{
class Identity;
}

class IdentityTester : public QObject
{
    Q_OBJECT

private:
    bool compareIdentities(const KIdentityManagement::Identity &actual, const KIdentityManagement::Identity &expected);

private Q_SLOTS:
    void initTestCase();
    void test_Identity();
    void test_NullIdentity();
    void test_Aliases();
    void test_toMimeData();
};

