# SPDX-FileCopyrightText: (C) 2024 Laurent Montel <montel@kde.org>
# SPDX-License-Identifier: BSD-3-Clause
########## next target ###############

add_executable(identitycombo_gui identitycombo_gui.cpp)

target_link_libraries(identitycombo_gui KPim6::IdentityManagementWidgets)
