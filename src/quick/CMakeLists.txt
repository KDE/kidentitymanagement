# SPDX-FileCopyrightText: (C) 2023 Claudio Cambra <claudio.cambra@kde.org>
#
# SPDX-License-Identifier: BSD-2-Clause

ecm_add_qml_module(kidentitymanagement_quick_plugin URI "org.kde.kidentitymanagement" VERSION 1.0)

target_sources(kidentitymanagement_quick_plugin PRIVATE
    kidentitymanagementquickplugin.cpp
    kidentitymanagementquickplugin.h
)

ecm_qt_declare_logging_category(kidentitymanagement_quick_plugin
    HEADER kidentitymanagement_quick_debug.h
    IDENTIFIER KIDENTITYMANAGEMENT_QUICK_LOG
    CATEGORY_NAME org.kde.kidentitymanagement.quick
    DESCRIPTION "KIdentityManagement QtQuick Plugin"
    EXPORT MERKURO
)

target_link_libraries(kidentitymanagement_quick_plugin PRIVATE
    Qt::Quick
    KPim${KF_MAJOR_VERSION}IdentityManagement
)

ecm_qt_install_logging_categories(
    EXPORT MERKURO
    FILE kidentitymanagement.quick.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

ecm_finalize_qml_module(kidentitymanagement_quick_plugin
    DESTINATION ${KDE_INSTALL_QMLDIR}
    BUILD_SHARED_LIBS OFF
)
