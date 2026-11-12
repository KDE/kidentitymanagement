/*
    SPDX-FileCopyrightText: 2016-2026 Laurent Montel <montel@kde.org>
    SPDX-FileCopyrightText: 2009 Thomas McGuire <mcguire@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QObject>

class SignatureTester : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testSignatures();
    void testTextEditInsertion();
    void testBug167961();
    void testImages();
    void testLinebreaks();
    void testEqualSignatures();
};
