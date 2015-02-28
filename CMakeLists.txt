cmake_minimum_required(VERSION 2.8.12)

project(KIdentityManagement)

# ECM setup
find_package(ECM 1.7.0 CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(GenerateExportHeader)
include(ECMGenerateHeaders)
include(ECMGeneratePriFile)
include(ECMPackageConfigHelpers)
include(ECMSetupVersion)
include(FeatureSummary)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings)


set(KF5_VERSION "5.6.0")
set(KIDENTITYMANAGEMENT_LIB_VERSION "4.79.0")
set(PIMTEXTEDIT_LIB_VERSION "4.79.0")

ecm_setup_version(${KIDENTITYMANAGEMENT_LIB_VERSION} VARIABLE_PREFIX KIDENTITYMANAGEMENT
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kidentitymanagement_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5IdentityManagementConfigVersion.cmake"
                        SOVERSION 5
)

########### Find packages ###########
#REMOVE IT WHEN WE REMOVE from KF5PimTextEdit 
find_package(KF5KDELibs4Support ${KF5_VERSION} CONFIG REQUIRED)

find_package(Qt5Network NO_MODULE REQUIRED)
find_package(KF5CoreAddons ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Completion ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5TextWidgets ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5XmlGui ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5KIO ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Config ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Emoticons ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Codecs ${KF5_VERSION} CONFIG REQUIRED)

find_package(KF5PimTextEdit ${PIMTEXTEDIT_LIB_VERSION} CONFIG REQUIRED)
add_definitions(-DTRANSLATION_DOMAIN=\"libkpimidentities5\")
add_definitions("-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII")

########### Targets ###########
add_subdirectory(src)
add_subdirectory(autotests)

########### CMake Config Files ###########
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5IdentityManagement")

ecm_configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5IdentityManagementConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5IdentityManagementConfig.cmake"
  INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF5IdentityManagementConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5IdentityManagementConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF5IdentityManagementTargets
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    FILE KF5IdentityManagementTargets.cmake
    NAMESPACE KF5::
)

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/kidentitymanagement_version.h
   DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}
   COMPONENT Devel
)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
