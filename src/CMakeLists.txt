# SPDX-FileCopyrightText: Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

add_subdirectory(core)
if(BUILD_QUICK_SUPPORT)
    add_subdirectory(quick)
endif()
add_subdirectory(widgets)
