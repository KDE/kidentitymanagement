// SPDX-FileCopyrightText: 2023 Claudio Cambra <claudio.cambra@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QObject>

#include <KIdentityManagementCore/IdentityManager>

class IdentityModelTester : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testModelCount();
    void testModelData();
    void testEmailFromUoid();

private:
    uint pretestIdentityCount = 0;
    /**
     * Initialise in constructor to ensure we are A) using test paths
     * so that we B) do not modify the user's real identity configs
     */
    std::unique_ptr<KIdentityManagementCore::IdentityManager> manager;
};
