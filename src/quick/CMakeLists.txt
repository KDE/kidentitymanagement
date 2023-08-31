# SPDX-FileCopyrightText: (C) 2023 Claudio Cambra <claudio.cambra@kde.org>
# SPDX-License-Identifier: BSD-2-Clause

## identitymanagentprivate target

ecm_setup_version(PROJECT VARIABLE_PREFIX KIDENTITYMANAGEMENTQUICK
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kidentitymanagementquick_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KPim6IdentityManagementQuickConfigVersion.cmake"
                        SOVERSION 6
)


add_library(identitymanagentprivate STATIC)

target_sources(identitymanagentprivate
    PRIVATE
        identityutils.h
        identityutils.cpp
        keylistmodelinterface.h
        identityeditorbackend.h
        identityeditorbackend.cpp
)

target_link_libraries(identitymanagentprivate
    PUBLIC
        Qt::Core
        Qt::Quick
        KPim${KF_MAJOR_VERSION}::IdentityManagement
    PRIVATE
        KF${KF_MAJOR_VERSION}::I18n
)

## KPimIdentityManagementQuick target

add_library(KPim${KF_MAJOR_VERSION}IdentityManagementQuick SHARED)
add_library(KPim${KF_MAJOR_VERSION}::IdentityManagementQuick ALIAS KPim${KF_MAJOR_VERSION}IdentityManagementQuick)

target_sources(KPim${KF_MAJOR_VERSION}IdentityManagementQuick
    PRIVATE
        cryptographybackendinterface.h
        cryptographyeditorbackend.h
        cryptographyeditorbackend.cpp
)


target_link_libraries(KPim${KF_MAJOR_VERSION}IdentityManagementQuick 
    PRIVATE
        identitymanagentprivate
)

if(COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(KPim${KF_MAJOR_VERSION}IdentityManagementQuick PROPERTIES UNITY_BUILD ON)
endif()

generate_export_header(KPim${KF_MAJOR_VERSION}IdentityManagementQuick
    BASE_NAME kidentitymanagementquick
)

target_include_directories(KPim${KF_MAJOR_VERSION}IdentityManagementQuick
    INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/KPim${KF_MAJOR_VERSION}/KIdentityManagementQuick>"
)
target_include_directories(KPim${KF_MAJOR_VERSION}IdentityManagementQuick PUBLIC "$<BUILD_INTERFACE:${KIdentityManagement_SOURCE_DIR}/src;${KIdentityManagement_BINARY_DIR}/src>")

set_target_properties(KPim${KF_MAJOR_VERSION}IdentityManagementQuick PROPERTIES
	VERSION ${KIDENTITYMANAGEMENTQUICK_VERSION}
	SOVERSION ${KIDENTITYMANAGEMENTQUICK_SOVERSION}
    EXPORT_NAME IdentityManagementQuick
)

install(TARGETS KPim${KF_MAJOR_VERSION}IdentityManagementQuick EXPORT KPim${KF_MAJOR_VERSION}IdentityManagementTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

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
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/kidentitymanagementquick_export.h
    ${KIdentityManagementQuick_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim${KF_MAJOR_VERSION}/KIdentityManagementQuick/kidentitymanagementquick
    COMPONENT Devel
)
install(FILES ${KIdentityManagementQuick_CamelCase_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim${KF_MAJOR_VERSION}/KIdentityManagementQuick/KIdentityManagementQuick/
    COMPONENT Devel
)

ecm_generate_pri_file(
    BASE_NAME KIdentityManagementQuick
    LIB_NAME KF${KF_MAJOR_VERSION}IdentityManagementQuick
    DEPS "KIdentityManagement"
    FILENAME_VAR PRI_FILENAME
    INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR}/KPim${KF_MAJOR_VERSION}/KIdentityManagementQuick)
install(FILES ${PRI_FILENAME}
    DESTINATION ${ECM_MKSPECS_INSTALL_DIR}
)

if(BUILD_QCH)
    ecm_add_qch(KPim${KF_MAJOR_VERSION}IdentityManagementQuick_QCH
        NAME
            KIdentityManagementQuick
        BASE_NAME
            KPim${KF_MAJOR_VERSION}IdentityManagementQuick
        VERSION
            ${PIM_VERSION}
        ORG_DOMAIN
            org.kde
        SOURCES # using only public headers, to cover only public API
            ${KIdentityManagementQuick_HEADERS}

        # MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        # IMAGE_DIRS "${CMAKE_SOURCE_DIR}/docs/pics"
        LINK_QCHS
            Qt${QT_MAJOR_VERSION}Quick_QCH
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
        BLANK_MACROS
            KIDENTITYMANAGEMENTQUICK_EXPORT
        TAGFILE_INSTALL_DESTINATION
            ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION
            ${KDE_INSTALL_QTQCHDIR}
        COMPONENT
            Devel
    )
endif()


# ########## QML Plugin s###############

ecm_add_qml_module(kidentitymanagement_quick_plugin URI "org.kde.kidentitymanagement" VERSION 1.0)

target_sources(kidentitymanagement_quick_plugin
    PRIVATE
        kidentitymanagementquickplugin.cpp
        kidentitymanagementquickplugin.h
)

target_link_libraries(kidentitymanagement_quick_plugin
    PUBLIC
        Qt::Core
        Qt::Quick
        identitymanagentprivate
        KPim${KF_MAJOR_VERSION}IdentityManagement
        KPim${KF_MAJOR_VERSION}IdentityManagementQuick
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

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/kidentitymanagementquick_version.h
   DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim${KF_MAJOR_VERSION}/KIdentityManagementQuick
   COMPONENT Devel
)

