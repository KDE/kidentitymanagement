# SPDX-FileCopyrightText: (C) 2023 Claudio Cambra <claudio.cambra@kde.org>
# SPDX-License-Identifier: BSD-2-Clause

## identitymanagentprivate target

ecm_setup_version(PROJECT VARIABLE_PREFIX KIDENTITYMANAGEMENTQUICK
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kidentitymanagementquick_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KPim6IdentityManagementQuickConfigVersion.cmake"
                        SOVERSION 6
)

add_library(identitymanagentprivate STATIC)

target_sources(
    identitymanagentprivate
    PRIVATE
        identityutils.h
        identityutils.cpp
        keylistmodelinterface.h
        identityeditorbackend.h
        identityeditorbackend.cpp
)
set_property(
    TARGET
        identitymanagentprivate
    PROPERTY
        POSITION_INDEPENDENT_CODE
            ON
)
target_link_libraries(
    identitymanagentprivate
    PUBLIC
        Qt::Core
        Qt::Quick
        KPim6::IdentityManagementCore
    PRIVATE
        KF6::I18n
)

## KPimIdentityManagementQuick target

add_library(KPim6IdentityManagementQuick SHARED)
add_library(KPim6::IdentityManagementQuick ALIAS KPim6IdentityManagementQuick)

target_sources(
    KPim6IdentityManagementQuick
    PRIVATE
        cryptographybackendinterface.h
        cryptographyeditorbackend.h
        cryptographyeditorbackend.cpp
)

target_link_libraries(KPim6IdentityManagementQuick PRIVATE identitymanagentprivate)

if(COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(
        KPim6IdentityManagementQuick
        PROPERTIES
            UNITY_BUILD
                ON
    )
endif()

generate_export_header(KPim6IdentityManagementQuick BASE_NAME kidentitymanagementquick)

target_include_directories(
    KPim6IdentityManagementQuick
    INTERFACE
        "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/KPim6/KIdentityManagementQuick>"
)
target_include_directories(
    KPim6IdentityManagementQuick
    PUBLIC
        "$<BUILD_INTERFACE:${KIdentityManagement_SOURCE_DIR}/src;${KIdentityManagement_BINARY_DIR}/src>"
)

set_target_properties(
    KPim6IdentityManagementQuick
    PROPERTIES
        VERSION
            ${KIDENTITYMANAGEMENTQUICK_VERSION}
        SOVERSION
            ${KIDENTITYMANAGEMENTQUICK_SOVERSION}
        EXPORT_NAME
            IdentityManagementQuick
)

set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KPim6IdentityManagementQuick")

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/KPim6IdentityManagementQuickConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/KPim6IdentityManagementQuickConfigVersion.cmake"
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    COMPONENT Devel
)

install(
    EXPORT KPim6IdentityManagementQuickTargets
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    FILE KPim6IdentityManagementQuickTargets.cmake
    NAMESPACE KPim6::
)

install(
    TARGETS
        KPim6IdentityManagementQuick
    EXPORT KPim6IdentityManagementQuickTargets
    ${KDE_INSTALL_TARGETS_DEFAULT_ARGS}
)

# ########## Generate Headers ###############
ecm_generate_headers(KIdentityManagementQuick_CamelCase_HEADERS
    HEADER_NAMES
        CryptographyBackendInterface
        CryptographyEditorBackend
        KeyListModelInterface
    PREFIX
        KIdentityManagementQuick
    REQUIRED_HEADERS
        KIdentityManagementQuick_HEADERS
)

# ########## install files ###############
install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/kidentitymanagementquick_export.h
        ${KIdentityManagementQuick_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim6/KIdentityManagementQuick/kidentitymanagementquick
    COMPONENT Devel
)
install(
    FILES
        ${KIdentityManagementQuick_CamelCase_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim6/KIdentityManagementQuick/KIdentityManagementQuick/
    COMPONENT Devel
)

# ########## QML Plugin s###############

ecm_add_qml_module(kidentitymanagement_quick_plugin URI "org.kde.kidentitymanagement" VERSION 1.0)

target_sources(
    kidentitymanagement_quick_plugin
    PRIVATE
        kidentitymanagementquickplugin.cpp
        kidentitymanagementquickplugin.h
)

target_link_libraries(
    kidentitymanagement_quick_plugin
    PUBLIC
        Qt::Core
        Qt::Quick
        identitymanagentprivate
        KPim6IdentityManagementCore
        KPim6IdentityManagementQuick
)

ecm_target_qml_sources(kidentitymanagement_quick_plugin SOURCES
    qml/BasicIdentityEditorCard.qml
    qml/CryptographyEditorCard.qml
    qml/IdentityConfigurationForm.qml
    qml/IdentityEditorPage.qml
)

ecm_finalize_qml_module(kidentitymanagement_quick_plugin
    DESTINATION ${KDE_INSTALL_QMLDIR}
    BUILD_SHARED_LIBS OFF
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/kidentitymanagementquick_version.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim6/KIdentityManagementQuick
    COMPONENT Devel
)
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/KPimIdentityManagementQuickConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/KPim6IdentityManagementQuickConfig.cmake"
    INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
)
