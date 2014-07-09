set(kpimidentities_SRCS
  identity.cpp
  identitycombo.cpp
  identitymanager.cpp
  signature.cpp
  signatureconfigurator.cpp
)

qt5_add_dbus_adaptor(kpimidentities_SRCS org.kde.pim.IdentityManager.xml identitymanager.h KPIMIdentities::IdentityManager )

add_library(KF5PimIdentities ${kpimidentities_SRCS})

generate_export_header(KF5PimIdentities BASE_NAME kpimidentities)

add_library(KF5::PimIdentities ALIAS KF5PimIdentities)

target_include_directories(KF5PimIdentities INTERFACE "$<INSTALL_INTERFACE:${KF5_INCLUDE_INSTALL_DIR}/KPIMIdentities;${KF5_INCLUDE_INSTALL_DIR}/KPIMIdentities/kpimidentities;>")
target_include_directories(KF5PimIdentities PUBLIC "$<BUILD_INTERFACE:${KPimIdentities_SOURCE_DIR}/src;${KPimIdentities_BINARY_DIR}/src>")

target_link_libraries(KF5PimIdentities
PUBLIC
  KF5::CoreAddons
  KF5::PimTextEdit
PRIVATE
  KF5::KIOCore
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

set_target_properties(KF5PimIdentities PROPERTIES
    VERSION ${KPIMIDENTITIES_VERSION_STRING}
    SOVERSION ${KPIMIDENTITIES_SOVERSION}
    EXPORT_NAME PimIdentities
)

install(TARGETS KF5PimIdentities EXPORT KF5PimIdentitiesTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

########### Generate Headers ###############
ecm_generate_headers(KPimIdentities_CamelCase_HEADERS
  HEADER_NAMES
  Identity
  IdentityCombo
  IdentityManager
  Signature
  SignatureConfigurator
  PREFIX KPIMIdentities
  REQUIRED_HEADERS KPimIdentities_HEADERS
)

########### install files ###############

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kpimidentities_export.h
  ${KPimIdentities_HEADERS}
    DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/KPIMIdentities/kpimidentities
    COMPONENT Devel
)
install(FILES    ${KPimIdentities_CamelCase_HEADERS}
    DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/KPIMIdentities/KPIMIdentities/
    COMPONENT Devel
)


install(FILES org.kde.pim.IdentityManager.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})

ecm_generate_pri_file(BASE_NAME KPimIdentities LIB_NAME KF5PimIdentities DEPS "CoreAddons PimTextEdit" FILENAME_VAR PRI_FILENAME  INCLUDE_INSTALL_DIR ${KF5_INCLUDE_INSTALL_DIR}/KPIMIdentities)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})
