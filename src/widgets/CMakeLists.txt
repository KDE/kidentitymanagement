# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause

ecm_setup_version(PROJECT VARIABLE_PREFIX KIDENTITYMANAGEMENTWIDGETS
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kidentitymanagementwidgets_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KPim6IdentityManagementWidgetsConfigVersion.cmake"
                        SOVERSION 6
)

add_library(KPim6IdentityManagementWidgets)
add_library(KPim6::IdentityManagementWidgets ALIAS KPim6IdentityManagementWidgets)
set_target_properties(
    KPim6IdentityManagementWidgets
    PROPERTIES
        VERSION
            ${KIDENTITYMANAGEMENTWIDGETS_VERSION}
        SOVERSION
            ${KIDENTITYMANAGEMENTWIDGETS_SOVERSION}
        EXPORT_NAME
            IdentityManagementWidgets
)

target_sources(
    KPim6IdentityManagementWidgets
    PRIVATE
        identitycombo.cpp
        signatureconfigurator.cpp
        signaturerichtexteditor.cpp
        signaturerichtexteditor_p.h
        identitycombo.h
        signatureconfigurator.h
        identitytreeview.h
        identitytreeview.cpp
        identitytreedelegate.h
        identitytreedelegate.cpp
)

if(COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(
        KPim6IdentityManagementWidgets
        PROPERTIES
            UNITY_BUILD
                ON
    )
endif()

generate_export_header(KPim6IdentityManagementWidgets BASE_NAME kidentitymanagementwidgets)

target_include_directories(
    KPim6IdentityManagementWidgets
    INTERFACE
        "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/KPim6/KIdentityManagementWidgets>"
)
target_include_directories(
    KPim6IdentityManagementWidgets
    PUBLIC
        "$<BUILD_INTERFACE:${KIdentityManagement_SOURCE_DIR}/src;${KIdentityManagement_BINARY_DIR}/src>"
)

target_link_libraries(
    KPim6IdentityManagementWidgets
    PUBLIC
        KPim6IdentityManagementCore
        Qt::Widgets
    PRIVATE
        KF6::CoreAddons
        KPim6::PimTextEdit
        KF6::Codecs
        Qt::Network
        KF6::I18n
        KF6::WidgetsAddons
        KF6::XmlGui
        KF6::KIOWidgets
        KF6::ConfigCore
        KF6::TextCustomEditor
)

########### CMake Config Files ###########
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KPim6IdentityManagementWidgets")

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/KPim6IdentityManagementWidgetsConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/KPim6IdentityManagementWidgetsConfigVersion.cmake"
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    COMPONENT Devel
)

install(
    EXPORT KPim6IdentityManagementWidgetsTargets
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    FILE KPim6IdentityManagementWidgetsTargets.cmake
    NAMESPACE KPim6::
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/kidentitymanagementwidgets_version.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim6/KIdentityManagementWidgets
    COMPONENT Devel
)

install(
    TARGETS
        KPim6IdentityManagementWidgets
    EXPORT KPim6IdentityManagementWidgetsTargets
    ${KDE_INSTALL_TARGETS_DEFAULT_ARGS}
)

########### Generate Headers ###############
ecm_generate_headers(KIdentityManagementWidgets_CamelCase_HEADERS
  HEADER_NAMES
  IdentityCombo
  SignatureConfigurator
  IdentityTreeView
  PREFIX KIdentityManagementWidgets
  REQUIRED_HEADERS KIdentityManagementWidgets_HEADERS
)

########### install files ###############

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/kidentitymanagementwidgets_export.h
        ${KIdentityManagementWidgets_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim6/KIdentityManagementWidgets/kidentitymanagementwidgets
    COMPONENT Devel
)
install(
    FILES
        ${KIdentityManagementWidgets_CamelCase_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim6/KIdentityManagementWidgets/KIdentityManagementWidgets/
    COMPONENT Devel
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/KPimIdentityManagementWidgetsConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/KPim6IdentityManagementWidgetsConfig.cmake"
    INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
)

if(BUILD_TESTING)
    add_subdirectory(tests)
    add_subdirectory(autotests)
endif()
