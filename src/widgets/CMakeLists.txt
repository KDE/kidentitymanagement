# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause
add_library(KPim6IdentityManagementWidgets)
add_library(KPim6::IdentityManagementWidgets ALIAS KPim6IdentityManagementWidgets)

target_sources(KPim6IdentityManagementWidgets PRIVATE
  identitycombo.cpp
  signatureconfigurator.cpp
  signaturerichtexteditor.cpp
  signaturerichtexteditor_p.h
  identitycombo.h
  signatureconfigurator.h
)

if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(KPim6IdentityManagementWidgets PROPERTIES UNITY_BUILD ON)
endif()

generate_export_header(KPim6IdentityManagementWidgets BASE_NAME kidentitymanagementwidgets)

target_include_directories(KPim6IdentityManagementWidgets INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/KPim6/KIdentityManagementWidgets>")
target_include_directories(KPim6IdentityManagementWidgets PUBLIC "$<BUILD_INTERFACE:${KIdentityManagement_SOURCE_DIR}/src;${KIdentityManagement_BINARY_DIR}/src>")

target_link_libraries(KPim6IdentityManagementWidgets
PUBLIC
  KPim6IdentityManagement
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
)

set_target_properties(KPim6IdentityManagementWidgets PROPERTIES
    VERSION ${KIDENTITYMANAGEMENT_VERSION}
    SOVERSION ${KIDENTITYMANAGEMENT_SOVERSION}
    EXPORT_NAME IdentityManagementWidgets
)

install(TARGETS KPim6IdentityManagementWidgets EXPORT KPim6IdentityManagementTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

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
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim6/KIdentityManagementWidgets/kidentitymanagement
    COMPONENT Devel
)
install(FILES ${KIdentityManagementWidgets_CamelCase_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim6/KIdentityManagementWidgets/KIdentityManagement/
    COMPONENT Devel
)

ecm_generate_pri_file(BASE_NAME KIdentityManagementWidgets LIB_NAME KF6IdentityManagementWidgets DEPS "KIdentityManagement KPimTextEdit"
    FILENAME_VAR PRI_FILENAME  INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR}/KPim6/KIdentityManagementWidgets)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})

if (BUILD_QCH)
    ecm_add_qch(
        KPim6IdentityManagementWidgets_QCH
        NAME KIdentityManagementWidgets
        BASE_NAME KPim6IdentityManagementWidgets
        VERSION ${PIM_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KIdentityManagementWidgets_HEADERS}
        #MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        #IMAGE_DIRS "${CMAKE_SOURCE_DIR}/docs/pics"
        LINK_QCHS
            Qt6Core_QCH
            Qt6Gui_QCH
            Qt6Widgets_QCH
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
        BLANK_MACROS
            KIDENTITYMANAGEMENTWIDGETS_EXPORT
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()
