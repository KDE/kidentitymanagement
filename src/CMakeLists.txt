add_library(KF5IdentityManagement)
add_library(KF5::IdentityManagement ALIAS KF5IdentityManagement)

qt_add_dbus_adaptor(kidentity_dbs_adaptor_SRCS org.kde.pim.IdentityManager.xml identitymanager.h KIdentityManagement::IdentityManager )

target_sources(KF5IdentityManagement PRIVATE
    ${kidentity_dbs_adaptor_SRCS}
  identity.cpp
  identitycombo.cpp
  identitymanager.cpp
  signature.cpp
  signatureconfigurator.cpp
  utils.cpp
)


ecm_qt_declare_logging_category(KF5IdentityManagement HEADER kidentitymanagement_debug.h
        IDENTIFIER KIDENTITYMANAGEMENT_LOG CATEGORY_NAME org.kde.pim.kidentitymanagement
        OLD_CATEGORY_NAMES log_kidentitymanagement
        DESCRIPTION "kidentitymanagement (pim lib)" EXPORT KIDENTITYMANAGEMENT)

if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(KF5IdentityManagement PROPERTIES UNITY_BUILD ON)
endif()

generate_export_header(KF5IdentityManagement BASE_NAME kidentitymanagement)



target_include_directories(KF5IdentityManagement INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/KIdentityManagement;${KDE_INSTALL_INCLUDEDIR_KF5}/KIdentityManagement/kidentitymanagement;>")
target_include_directories(KF5IdentityManagement PUBLIC "$<BUILD_INTERFACE:${KIdentityManagement_SOURCE_DIR}/src;${KIdentityManagement_BINARY_DIR}/src>")

target_link_libraries(KF5IdentityManagement
PUBLIC
  KF5::CoreAddons
  KF5::PimTextEdit
PRIVATE
  KF5::Codecs
  Qt::Network
  KF5::I18n
  KF5::WidgetsAddons
  KF5::XmlGui
  KF5::KIOWidgets
  KF5::ConfigCore
)

set_target_properties(KF5IdentityManagement PROPERTIES
    VERSION ${KIDENTITYMANAGEMENT_VERSION}
    SOVERSION ${KIDENTITYMANAGEMENT_SOVERSION}
    EXPORT_NAME IdentityManagement
)

install(TARGETS KF5IdentityManagement EXPORT KF5IdentityManagementTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

########### Generate Headers ###############
ecm_generate_headers(KIdentityManagement_CamelCase_HEADERS
  HEADER_NAMES
  Identity
  IdentityCombo
  IdentityManager
  Signature
  SignatureConfigurator
  Utils
  PREFIX KIdentityManagement
  REQUIRED_HEADERS KIdentityManagement_HEADERS
)

########### install files ###############

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kidentitymanagement_export.h
  ${KIdentityManagement_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/KIdentityManagement/kidentitymanagement
    COMPONENT Devel
)
install(FILES    ${KIdentityManagement_CamelCase_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/KIdentityManagement/KIdentityManagement/
    COMPONENT Devel
)


install(FILES org.kde.pim.IdentityManager.xml DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR} RENAME kf5_org.kde.pim.IdentityManager.xml)

ecm_generate_pri_file(BASE_NAME KIdentityManagement LIB_NAME KF5IdentityManagement DEPS "KCoreAddons KPimTextEdit" FILENAME_VAR PRI_FILENAME  INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF5}/KIdentityManagement)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})

ecm_qt_install_logging_categories(EXPORT KIDENTITYMANAGEMENT FILE kidentitymanagement.categories DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR})
