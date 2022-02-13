# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause
add_library(KF5IdentityManagementWidgets)
add_library(KF5::IdentityManagementWidgets ALIAS KF5IdentityManagementWidgets)

target_sources(KF5IdentityManagementWidgets PRIVATE
  identitycombo.cpp
  signatureconfigurator.cpp
  signaturerichtexteditor.cpp
  signaturerichtexteditor_p.h
  identitycombo.h
  signatureconfigurator.h
)

if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(KF5IdentityManagementWidgets PROPERTIES UNITY_BUILD ON)
endif()

generate_export_header(KF5IdentityManagementWidgets BASE_NAME kidentitymanagementwidgets)

target_include_directories(KF5IdentityManagementWidgets INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KIdentityManagementWidgets;${KDE_INSTALL_INCLUDEDIR_KF}/KIdentityManagementWidgets/kidentitymanagement;>")
target_include_directories(KF5IdentityManagementWidgets PUBLIC "$<BUILD_INTERFACE:${KIdentityManagement_SOURCE_DIR}/src;${KIdentityManagement_BINARY_DIR}/src>")

target_link_libraries(KF5IdentityManagementWidgets
PUBLIC
  KF5IdentityManagement
  Qt${QT_MAJOR_VERSION}::Widgets
PRIVATE
  KF5::CoreAddons
  KF5::PimTextEdit
  KF5::Codecs
  Qt${QT_MAJOR_VERSION}::Network
  KF5::I18n
  KF5::WidgetsAddons
  KF5::XmlGui
  KF5::KIOWidgets
  KF5::ConfigCore
)

set_target_properties(KF5IdentityManagementWidgets PROPERTIES
    VERSION ${KIDENTITYMANAGEMENT_VERSION}
    SOVERSION ${KIDENTITYMANAGEMENT_SOVERSION}
    EXPORT_NAME IdentityManagementWidgets
)

install(TARGETS KF5IdentityManagementWidgets EXPORT KF5IdentityManagementTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

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
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KIdentityManagementWidgets/kidentitymanagement
    COMPONENT Devel
)
install(FILES ${KIdentityManagementWidgets_CamelCase_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KIdentityManagementWidgets/KIdentityManagement/
    COMPONENT Devel
)

ecm_generate_pri_file(BASE_NAME KIdentityManagementWidgets LIB_NAME KF5IdentityManagementWidgets DEPS "KIdentityManagement KPimTextEdit" FILENAME_VAR PRI_FILENAME  INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF}/KIdentityManagementWidgets)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})

if (BUILD_QCH)
    ecm_add_qch(
        KF5IdentityManagementWidgets_QCH
        NAME KIdentityManagementWidgets
        BASE_NAME KF5IdentityManagementWidgets
        VERSION ${PIM_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KIdentityManagementWidgets_HEADERS}
        #MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        #IMAGE_DIRS "${CMAKE_SOURCE_DIR}/docs/pics"
        LINK_QCHS
            Qt5Core_QCH
            Qt5Gui_QCH
            Qt5Widgets_QCH
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
        BLANK_MACROS
            KIDENTITYMANAGEMENTWIDGETS_EXPORT
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()
