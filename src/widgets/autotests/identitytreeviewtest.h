// SPDX-FileCopyrightText: 2024-2026 Laurent Montel <montel@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#pragma once

#include <QObject>

class IdentityTreeViewTest : public QObject
{
    Q_OBJECT
public:
    explicit IdentityTreeViewTest(QObject *parent = nullptr);
    ~IdentityTreeViewTest() override = default;
private Q_SLOTS:
    void shouldHaveDefaultValues();
};
