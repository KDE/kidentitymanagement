# SPDX-FileCopyrightText: (C) 2023 Claudio Cambra <claudio.cambra@kde.org>
# SPDX-License-Identifier: BSD-2-Clause
add_library(KPim${KF_MAJOR_VERSION}IdentityManagementQuick)
add_library(KPim${KF_MAJOR_VERSION}::IdentityManagementQuick ALIAS KPim${KF_MAJOR_VERSION}IdentityManagementQuick)

set(QUICK_SRCS
    PRIVATE
    cryptographybackendinterface.h
    cryptographyeditorbackend.h
    cryptographyeditorbackend.cpp
    identityeditorbackend.h
    identityeditorbackend.cpp
    identityutils.h
    identityutils.cpp
    kidentitymanagementquickplugin.cpp
    kidentitymanagementquickplugin.h
    keylistmodelinterface.h
)

set(QUICK_LIBS
    PRIVATE
    Qt::Core
    Qt::Quick
    KF${KF_MAJOR_VERSION}::I18n
    KPim${KF_MAJOR_VERSION}IdentityManagement
)

if(COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(KPim${KF_MAJOR_VERSION}IdentityManagementQuick PROPERTIES UNITY_BUILD ON)
endif()

generate_export_header(KPim${KF_MAJOR_VERSION}IdentityManagementQuick BASE_NAME kidentitymanagementquick)

target_include_directories(KPim${KF_MAJOR_VERSION}IdentityManagementQuick INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/KPim${KF_MAJOR_VERSION}/KIdentityManagementQuick>")
target_include_directories(KPim${KF_MAJOR_VERSION}IdentityManagementQuick PUBLIC "$<BUILD_INTERFACE:${KIdentityManagement_SOURCE_DIR}/src;${KIdentityManagement_BINARY_DIR}/src>")

target_sources(KPim${KF_MAJOR_VERSION}IdentityManagementQuick PRIVATE ${QUICK_SRCS})
target_link_libraries(KPim${KF_MAJOR_VERSION}IdentityManagementQuick PRIVATE ${QUICK_LIBS})

set_target_properties(KPim${KF_MAJOR_VERSION}IdentityManagementQuick PROPERTIES
    VERSION ${KIDENTITYMANAGEMENT_VERSION}
    SOVERSION ${KIDENTITYMANAGEMENT_SOVERSION}
    EXPORT_NAME IdentityManagementQuick
)

install(TARGETS KPim${KF_MAJOR_VERSION}IdentityManagementQuick EXPORT KPim${KF_MAJOR_VERSION}IdentityManagementTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

ecm_add_qml_module(kidentitymanagement_quick_plugin URI "org.kde.kidentitymanagement" VERSION 1.0)

target_sources(kidentitymanagement_quick_plugin PRIVATE ${QUICK_SRCS})
target_link_libraries(kidentitymanagement_quick_plugin PRIVATE ${QUICK_LIBS})

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

# ########## Generate Headers ###############
ecm_generate_headers(KIdentityManagementQuick_CamelCase_HEADERS
    HEADER_NAMES CryptographyBackendInterface CryptographyEditorBackend KeyListModelInterface
    PREFIX KIdentityManagement
    REQUIRED_HEADERS KIdentityManagementQuick_HEADERS
)

# ########## install files ###############
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/kidentitymanagementquick_export.h
    ${KIdentityManagementQuick_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim${KF_MAJOR_VERSION}/KIdentityManagementQuick/kidentitymanagement
    COMPONENT Devel
)
install(FILES ${KIdentityManagementQuick_CamelCase_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim${KF_MAJOR_VERSION}/KIdentityManagementQuick/KIdentityManagement/
    COMPONENT Devel
)

ecm_generate_pri_file(BASE_NAME KIdentityManagementQuick LIB_NAME KF${KF_MAJOR_VERSION}IdentityManagementQuick DEPS "KIdentityManagement"
    FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR}/KPim${KF_MAJOR_VERSION}/KIdentityManagementQuick)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})

if(BUILD_QCH)
    ecm_add_qch(
        KPim${KF_MAJOR_VERSION}IdentityManagementQuick_QCH
        NAME KIdentityManagementQuick
        BASE_NAME KPim${KF_MAJOR_VERSION}IdentityManagementQuick
        VERSION ${PIM_VERSION}
        ORG_DOMAIN org.kde
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
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()
