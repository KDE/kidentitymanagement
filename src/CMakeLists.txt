set(kidentitymanagement_SRCS
  identity.cpp
  identitycombo.cpp
  identitymanager.cpp
  signature.cpp
  signatureconfigurator.cpp
)

qt5_add_dbus_adaptor(kidentitymanagement_SRCS org.kde.pim.IdentityManager.xml identitymanager.h KIdentityManagement::IdentityManager )

add_library(KF5IdentityManagement ${kidentitymanagement_SRCS})

generate_export_header(KF5IdentityManagement BASE_NAME kidentitymanagement)

add_library(KF5::IdentityManagement ALIAS KF5IdentityManagement)

target_include_directories(KF5IdentityManagement INTERFACE "$<INSTALL_INTERFACE:${KF5_INCLUDE_INSTALL_DIR}/KIdentityManagement;${KF5_INCLUDE_INSTALL_DIR}/KIdentityManagement/kidentitymanagement;>")
target_include_directories(KF5IdentityManagement PUBLIC "$<BUILD_INTERFACE:${KIdentityManagement_SOURCE_DIR}/src;${KIdentityManagement_BINARY_DIR}/src>")

target_link_libraries(KF5IdentityManagement
PUBLIC
  KF5::CoreAddons
  KF5::PimTextEdit
PRIVATE
  KF5::PimUtils
  Qt5::Network
  KF5::Completion
  KF5::I18n
  KF5::TextWidgets
  KF5::WidgetsAddons
  KF5::XmlGui
  KF5::KIOWidgets
  KF5::ConfigCore
)

set_target_properties(KF5IdentityManagement PROPERTIES
    VERSION ${KIDENTITYMANAGEMENT_VERSION_STRING}
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
  PREFIX KIdentityManagement
  REQUIRED_HEADERS KIdentityManagement_HEADERS
)

########### install files ###############

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kidentitymanagement_export.h
  ${KIdentityManagement_HEADERS}
    DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/KIdentityManagement/kidentitymanagement
    COMPONENT Devel
)
install(FILES    ${KIdentityManagement_CamelCase_HEADERS}
    DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/KIdentityManagement/KIdentityManagement/
    COMPONENT Devel
)


install(FILES org.kde.pim.IdentityManager.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})

ecm_generate_pri_file(BASE_NAME KIdentityManagement LIB_NAME KF5IdentityManagement DEPS "CoreAddons PimTextEdit" FILENAME_VAR PRI_FILENAME  INCLUDE_INSTALL_DIR ${KF5_INCLUDE_INSTALL_DIR}/KIdentityManagement)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})
