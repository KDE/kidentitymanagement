cmake_minimum_required(VERSION 2.8.12)

project(KPimIdentities)

# ECM setup
find_package(ECM 1.1.0 CONFIG REQUIRED)
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


set(KF5_VERSION "5.1.0")
set(KDEPIMLIBS_VERSION "4.70.0")

ecm_setup_version(${KDEPIMLIBS_VERSION} VARIABLE_PREFIX KPIMIDENTITIES
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kpimidentities_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5PimIdentitiesConfigVersion.cmake"
                        SOVERSION 5
)

########### Find packages ###########
#REMOVE IT WHEN WE REMOVE from KF5PimTextEdit or KF5PimUtils
find_package(KF5KDELibs4Support ${KF5_VERSION} CONFIG REQUIRED)

find_package(Qt5Network NO_MODULE REQUIRED)
find_package(KF5CoreAddons ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5KIO ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Completion ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5TextWidgets ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5XmlGui ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5KIO ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Config ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Emoticons ${KF5_VERSION} CONFIG REQUIRED)

if("${CMAKE_SOURCE_DIR}" STREQUAL "${KPimIdentities_SOURCE_DIR}")
  find_package(KF5PimTextEdit ${KDEPIMLIBS_VERSION} CONFIG REQUIRED)
  find_package(KF5PimUtils ${KDEPIMLIBS_VERSION} CONFIG REQUIRED)
endif()
add_definitions(-DTRANSLATION_DOMAIN=\"libkpimidentities5\")
add_definitions("-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII")

########### Targets ###########
add_subdirectory(src)
add_subdirectory(autotests)

########### CMake Config Files ###########
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KF5PimIdentities")

ecm_configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5PimIdentitiesConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5PimIdentitiesConfig.cmake"
  INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF5PimIdentitiesConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5PimIdentitiesConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF5PimIdentitiesTargets
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    FILE KF5PimIdentitiesTargets.cmake
    NAMESPACE KF5::
)

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/kpimidentities_version.h
   DESTINATION ${KF5_INCLUDE_INSTALL_DIR}
   COMPONENT Devel
)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${KPimIdentities_BINARY_DIR}")
  feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
