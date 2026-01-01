# SPDX-FileCopyrightText: (C) 2024-2026 Laurent Montel <montel@kde.org>
# SPDX-License-Identifier: BSD-3-Clause
########## next target ###############

add_executable(identitycombo_gui identitycombo_gui.cpp)

target_link_libraries(identitycombo_gui KPim6::IdentityManagementWidgets)

########## next target ###############

add_executable(identitytreeview_gui identitytreeview_gui.cpp)

target_link_libraries(identitytreeview_gui KPim6::IdentityManagementWidgets)
