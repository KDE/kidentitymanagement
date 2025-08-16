# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause
ecm_setup_version(PROJECT VARIABLE_PREFIX KIDENTITYMANAGEMENTCORE
    VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kidentitymanagementcore_version.h"
    PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KPim6IdentityManagementCoreConfigVersion.cmake"
    SOVERSION 6
)

add_library(KPim6IdentityManagementCore)
add_library(KPim6::IdentityManagementCore ALIAS KPim6IdentityManagementCore)

qt_add_dbus_adaptor(kidentity_dbs_adaptor_SRCS org.kde.pim.IdentityManager.xml identitymanager.h KIdentityManagementCore::IdentityManager)

target_sources(
    KPim6IdentityManagementCore
    PRIVATE
        ${kidentity_dbs_adaptor_SRCS}
        identity.cpp
        identitymanager.cpp
        identitymodel.cpp
        signature.cpp
        utils.cpp
        utils.h
        signature.h
        identitymanager.h
        identity.h
        identitymodel.h
        identitytreemodel.h
        identitytreemodel.cpp
        identitytreesortproxymodel.h
        identitytreesortproxymodel.cpp
        identityactivitiesabstract.h
        identityactivitiesabstract.cpp
)

ecm_qt_declare_logging_category(KPim6IdentityManagementCore HEADER kidentitymanagementcore_debug.h
    IDENTIFIER KIDENTITYMANAGEMENT_LOG CATEGORY_NAME org.kde.pim.kidentitymanagement
    OLD_CATEGORY_NAMES log_kidentitymanagement
    DESCRIPTION "kidentitymanagement (pim lib)" EXPORT KIDENTITYMANAGEMENT
)

if(COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(
        KPim6IdentityManagementCore
        PROPERTIES
            UNITY_BUILD
                ON
    )
endif()

generate_export_header(KPim6IdentityManagementCore BASE_NAME kidentitymanagementcore)

target_include_directories(
    KPim6IdentityManagementCore
    INTERFACE
        "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/KPim6/KIdentityManagementCore>"
)
target_include_directories(
    KPim6IdentityManagementCore
    PUBLIC
        "$<BUILD_INTERFACE:${KIdentityManagement_SOURCE_DIR}/src/>"
)

target_link_libraries(
    KPim6IdentityManagementCore
    PUBLIC
        Qt::Gui
    PRIVATE
        KF6::CoreAddons
        KF6::ConfigCore
        KF6::Codecs
        KF6::I18n
        Qt::DBus
        Qt::Network
)

set_target_properties(
    KPim6IdentityManagementCore
    PROPERTIES
        VERSION
            ${KIDENTITYMANAGEMENTCORE_VERSION}
        SOVERSION
            ${KIDENTITYMANAGEMENTCORE_SOVERSION}
        EXPORT_NAME
            IdentityManagementCore
)

########### CMake Config Files ###########
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KPim6IdentityManagementCore")

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/KPim6IdentityManagementCoreConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/KPim6IdentityManagementCoreConfigVersion.cmake"
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    COMPONENT Devel
)

install(
    EXPORT KPim6IdentityManagementCoreTargets
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    FILE KPim6IdentityManagementCoreTargets.cmake
    NAMESPACE KPim6::
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/kidentitymanagementcore_version.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim6/KIdentityManagementCore
    COMPONENT Devel
)

install(
    TARGETS
        KPim6IdentityManagementCore
    EXPORT KPim6IdentityManagementCoreTargets
    ${KDE_INSTALL_TARGETS_DEFAULT_ARGS}
)

########### Generate Headers ###############
ecm_generate_headers(KIdentityManagementCore_CamelCase_HEADERS
    HEADER_NAMES
    Identity
    IdentityManager
    IdentityModel
    Signature
    Utils
    IdentityTreeModel
    IdentityActivitiesAbstract
    IdentityTreeSortProxyModel
    PREFIX KIdentityManagementCore
    REQUIRED_HEADERS KIdentityManagementCore_HEADERS
)

########### install files ###############

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/kidentitymanagementcore_export.h
        ${KIdentityManagementCore_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim6/KIdentityManagementCore/kidentitymanagementcore
    COMPONENT Devel
)
install(
    FILES
        ${KIdentityManagementCore_CamelCase_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim6/KIdentityManagementCore/KIdentityManagementCore/
    COMPONENT Devel
)

install(
    FILES
        org.kde.pim.IdentityManager.xml
    DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR}
    RENAME kf6_org.kde.pim.IdentityManager.xml
)

ecm_qt_install_logging_categories(EXPORT KIDENTITYMANAGEMENT FILE kidentitymanagement.categories DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR})

if(BUILD_QCH)
    ecm_add_qch(
        KPim6IdentityManagementCore_QCH
        NAME KIdentityManagementCore
        BASE_NAME KPim6IdentityManagementCore
        VERSION ${PIM_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
        ${KIdentityManagementCore_HEADERS}
        LINK_QCHS
        Qt6Gui_QCH
        INCLUDE_DIRS
        ${CMAKE_CURRENT_BINARY_DIR}
        BLANK_MACROS
        KIDENTITYMANAGEMENTCORE_EXPORT
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()

if(BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KPim6IdentityManagementCore_QCH
        FILE KPim6IdentityManagementCoreQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS
        "include(\"\${CMAKE_CURRENT_LIST_DIR}/KPim6IdentityManagementCoreQchTargets.cmake\")"
    )
endif()
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/KPimIdentityManagementCoreConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/KPim6IdentityManagementCoreConfig.cmake"
    INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
)

if(BUILD_TESTING)
    add_subdirectory(autotests)
endif()
