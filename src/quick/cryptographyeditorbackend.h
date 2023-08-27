// SPDX-FileCopyrightText: 2023 Claudio Cambra <claudio.cambra@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QObject>

namespace KIdentityManagement
{
namespace Quick
{

class CryptographyEditorBackend : public QObject
{
    Q_OBJECT
public:
    explicit CryptographyEditorBackend(QObject *parent = nullptr);
};

}
}
