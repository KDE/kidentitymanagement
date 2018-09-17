cmake_minimum_required(VERSION 3.0)
set(PIM_VERSION "5.9.40")

project(KIdentityManagement VERSION ${PIM_VERSION})

# ECM setup
set(KF5_VERSION "5.50.0")

find_package(ECM ${KF5_VERSION} CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(GenerateExportHeader)
include(ECMGenerateHeaders)
include(ECMGeneratePriFile)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)
include(FeatureSummary)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(ECMQtDeclareLoggingCategory)
include(ECMCoverageOption)



set(KIDENTITYMANAGEMENT_LIB_VERSION ${PIM_VERSION})
set(PIMTEXTEDIT_LIB_VERSION "5.9.40")

ecm_setup_version(PROJECT VARIABLE_PREFIX KIDENTITYMANAGEMENT
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kidentitymanagement_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5IdentityManagementConfigVersion.cmake"
                        SOVERSION 5
)

########### Find packages ###########

find_package(Qt5Network NO_MODULE REQUIRED)
find_package(KF5CoreAddons ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Completion ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5TextWidgets ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5XmlGui ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5KIO ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Config ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Codecs ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5IconThemes ${KF5_VERSION} CONFIG REQUIRED)

find_package(KF5PimTextEdit ${PIMTEXTEDIT_LIB_VERSION} CONFIG REQUIRED)
add_definitions(-DTRANSLATION_DOMAIN=\"libkpimidentities5\")
add_definitions(-DQT_NO_NARROWING_CONVERSIONS_IN_CONNECT)
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x060000)

########### Targets ###########
add_subdirectory(src)

if(BUILD_TESTING)
    add_subdirectory(autotests)
endif()

########### CMake Config Files ###########
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5IdentityManagement")

configure_package_config_file(
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

install( FILES kidentitymanagement.categories kidentitymanagement.renamecategories DESTINATION ${KDE_INSTALL_CONFDIR} )

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
