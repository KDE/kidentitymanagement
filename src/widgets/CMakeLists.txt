# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause
add_library(KPim${KF_MAJOR_VERSION}IdentityManagementWidgets)
add_library(KPim${KF_MAJOR_VERSION}::IdentityManagementWidgets ALIAS KPim${KF_MAJOR_VERSION}IdentityManagementWidgets)
add_library(KF5::IdentityManagementWidgets ALIAS KPim${KF_MAJOR_VERSION}IdentityManagementWidgets)

target_sources(KPim${KF_MAJOR_VERSION}IdentityManagementWidgets PRIVATE
  identitycombo.cpp
  signatureconfigurator.cpp
  signaturerichtexteditor.cpp
  signaturerichtexteditor_p.h
  identitycombo.h
  signatureconfigurator.h
)

if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(KPim${KF_MAJOR_VERSION}IdentityManagementWidgets PROPERTIES UNITY_BUILD ON)
endif()

generate_export_header(KPim${KF_MAJOR_VERSION}IdentityManagementWidgets BASE_NAME kidentitymanagementwidgets)

target_include_directories(KPim${KF_MAJOR_VERSION}IdentityManagementWidgets INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/KPim${KF_MAJOR_VERSION}/KIdentityManagementWidgets>")
target_include_directories(KPim${KF_MAJOR_VERSION}IdentityManagementWidgets PUBLIC "$<BUILD_INTERFACE:${KIdentityManagement_SOURCE_DIR}/src;${KIdentityManagement_BINARY_DIR}/src>")

target_link_libraries(KPim${KF_MAJOR_VERSION}IdentityManagementWidgets
PUBLIC
  KPim${KF_MAJOR_VERSION}IdentityManagement
  Qt::Widgets
PRIVATE
  KF${KF_MAJOR_VERSION}::CoreAddons
  KPim${KF_MAJOR_VERSION}::PimTextEdit
  KF${KF_MAJOR_VERSION}::Codecs
  Qt::Network
  KF${KF_MAJOR_VERSION}::I18n
  KF${KF_MAJOR_VERSION}::WidgetsAddons
  KF${KF_MAJOR_VERSION}::XmlGui
  KF${KF_MAJOR_VERSION}::KIOWidgets
  KF${KF_MAJOR_VERSION}::ConfigCore
)

set_target_properties(KPim${KF_MAJOR_VERSION}IdentityManagementWidgets PROPERTIES
    VERSION ${KIDENTITYMANAGEMENT_VERSION}
    SOVERSION ${KIDENTITYMANAGEMENT_SOVERSION}
    EXPORT_NAME IdentityManagementWidgets
)

install(TARGETS KPim${KF_MAJOR_VERSION}IdentityManagementWidgets EXPORT KPim${KF_MAJOR_VERSION}IdentityManagementTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

########### Generate Headers ###############
ecm_generate_headers(KIdentityManagementWidgets_CamelCase_HEADERS
  HEADER_NAMES
  IdentityCombo
  SignatureConfigurator
  PREFIX KIdentityManagement
  REQUIRED_HEADERS KIdentityManagementWidgets_HEADERS
)

########### install files ###############

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kidentitymanagementwidgets_export.h
  ${KIdentityManagementWidgets_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim${KF_MAJOR_VERSION}/KIdentityManagementWidgets/kidentitymanagement
    COMPONENT Devel
)
install(FILES ${KIdentityManagementWidgets_CamelCase_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim${KF_MAJOR_VERSION}/KIdentityManagementWidgets/KIdentityManagement/
    COMPONENT Devel
)

ecm_generate_pri_file(BASE_NAME KIdentityManagementWidgets LIB_NAME KF${KF_MAJOR_VERSION}IdentityManagementWidgets DEPS "KIdentityManagement KPimTextEdit"
    FILENAME_VAR PRI_FILENAME  INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR}/KPim${KF_MAJOR_VERSION}/KIdentityManagementWidgets)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})

if (BUILD_QCH)
    ecm_add_qch(
        KPim${KF_MAJOR_VERSION}IdentityManagementWidgets_QCH
        NAME KIdentityManagementWidgets
        BASE_NAME KPim${KF_MAJOR_VERSION}IdentityManagementWidgets
        VERSION ${PIM_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KIdentityManagementWidgets_HEADERS}
        #MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        #IMAGE_DIRS "${CMAKE_SOURCE_DIR}/docs/pics"
        LINK_QCHS
            Qt${QT_MAJOR_VERSION}Core_QCH
            Qt${QT_MAJOR_VERSION}Gui_QCH
            Qt${QT_MAJOR_VERSION}Widgets_QCH
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
        BLANK_MACROS
            KIDENTITYMANAGEMENTWIDGETS_EXPORT
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()
