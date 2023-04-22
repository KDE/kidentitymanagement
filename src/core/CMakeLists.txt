# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause
add_library(KPim${KF_MAJOR_VERSION}IdentityManagement)
add_library(KPim${KF_MAJOR_VERSION}::IdentityManagement ALIAS KPim${KF_MAJOR_VERSION}IdentityManagement)
add_library(KF5::IdentityManagement ALIAS KPim${KF_MAJOR_VERSION}IdentityManagement)


qt_add_dbus_adaptor(kidentity_dbs_adaptor_SRCS org.kde.pim.IdentityManager.xml identitymanager.h KIdentityManagement::IdentityManager )

target_sources(KPim${KF_MAJOR_VERSION}IdentityManagement PRIVATE
    ${kidentity_dbs_adaptor_SRCS}
  identity.cpp
  identitymanager.cpp
  signature.cpp
  utils.cpp
  utils.h
  signature.h
  identitymanager.h
  identity.h
)

ecm_qt_declare_logging_category(KPim${KF_MAJOR_VERSION}IdentityManagement HEADER kidentitymanagement_debug.h
        IDENTIFIER KIDENTITYMANAGEMENT_LOG CATEGORY_NAME org.kde.pim.kidentitymanagement
        OLD_CATEGORY_NAMES log_kidentitymanagement
        DESCRIPTION "kidentitymanagement (pim lib)" EXPORT KIDENTITYMANAGEMENT)

if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(KPim${KF_MAJOR_VERSION}IdentityManagement PROPERTIES UNITY_BUILD ON)
endif()

generate_export_header(KPim${KF_MAJOR_VERSION}IdentityManagement BASE_NAME kidentitymanagement)

target_include_directories(KPim${KF_MAJOR_VERSION}IdentityManagement INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/KPim${KF_MAJOR_VERSION}/KIdentityManagement>")
target_include_directories(KPim${KF_MAJOR_VERSION}IdentityManagement PUBLIC "$<BUILD_INTERFACE:${KIdentityManagement_SOURCE_DIR}/src;${KIdentityManagement_BINARY_DIR}/src>")

target_link_libraries(KPim${KF_MAJOR_VERSION}IdentityManagement
PUBLIC
  Qt::Gui
PRIVATE
  KF${KF_MAJOR_VERSION}::CoreAddons
  KF${KF_MAJOR_VERSION}::ConfigCore
  KF${KF_MAJOR_VERSION}::Codecs
  KF${KF_MAJOR_VERSION}::I18n
  Qt::DBus
  Qt::Network
)

set_target_properties(KPim${KF_MAJOR_VERSION}IdentityManagement PROPERTIES
    VERSION ${KIDENTITYMANAGEMENT_VERSION}
    SOVERSION ${KIDENTITYMANAGEMENT_SOVERSION}
    EXPORT_NAME IdentityManagement
)

install(TARGETS KPim${KF_MAJOR_VERSION}IdentityManagement EXPORT KPim${KF_MAJOR_VERSION}IdentityManagementTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

########### Generate Headers ###############
ecm_generate_headers(KIdentityManagement_CamelCase_HEADERS
  HEADER_NAMES
  Identity
  IdentityManager
  Signature
  Utils
  PREFIX KIdentityManagement
  REQUIRED_HEADERS KIdentityManagement_HEADERS
)

########### install files ###############

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kidentitymanagement_export.h
  ${KIdentityManagement_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim${KF_MAJOR_VERSION}/KIdentityManagement/kidentitymanagement
    COMPONENT Devel
)
install(FILES    ${KIdentityManagement_CamelCase_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim${KF_MAJOR_VERSION}/KIdentityManagement/KIdentityManagement/
    COMPONENT Devel
)


install(FILES org.kde.pim.IdentityManager.xml DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR} RENAME kf5_org.kde.pim.IdentityManager.xml)

ecm_generate_pri_file(BASE_NAME KIdentityManagement LIB_NAME KF${KF_MAJOR_VERSION}IdentityManagement DEPS "KCoreAddons KPimTextEdit"
    FILENAME_VAR PRI_FILENAME  INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR}/KPim${KF_MAJOR_VERSION}/KIdentityManagement)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})

ecm_qt_install_logging_categories(EXPORT KIDENTITYMANAGEMENT FILE kidentitymanagement.categories DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR})

if (BUILD_QCH)
    ecm_add_qch(
        KPim${KF_MAJOR_VERSION}IdentityManagement_QCH
        NAME KIdentityManagement
        BASE_NAME KPim${KF_MAJOR_VERSION}IdentityManagement
        VERSION ${PIM_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KIdentityManagement_HEADERS}
        #MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        #IMAGE_DIRS "${CMAKE_SOURCE_DIR}/docs/pics"
        LINK_QCHS
            Qt${QT_MAJOR_VERSION}Core_QCH
            Qt${QT_MAJOR_VERSION}Gui_QCH
            Qt${QT_MAJOR_VERSION}Widgets_QCH
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
        BLANK_MACROS
            KIDENTITYMANAGEMENT_EXPORT
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()
