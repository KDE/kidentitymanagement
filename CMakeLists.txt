cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
set(PIM_VERSION "5.18.1")

project(KIdentityManagement VERSION ${PIM_VERSION})

# ECM setup
set(KF5_MIN_VERSION "5.83.0")
set(QT_REQUIRED_VERSION "5.15.0")
find_package(ECM ${KF5_MIN_VERSION} CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)

include(GenerateExportHeader)
include(ECMGenerateHeaders)
include(ECMGeneratePriFile)

include(ECMSetupVersion)
include(FeatureSummary)
include(KDEGitCommitHooks)
include(ECMQtDeclareLoggingCategory)




set(KIDENTITYMANAGEMENT_LIB_VERSION ${PIM_VERSION})
set(PIMTEXTEDIT_LIB_VERSION "5.18.1")

ecm_setup_version(PROJECT VARIABLE_PREFIX KIDENTITYMANAGEMENT
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kidentitymanagement_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5IdentityManagementConfigVersion.cmake"
                        SOVERSION 5
)

########### Find packages ###########

find_package(Qt5Network NO_MODULE REQUIRED)
find_package(KF5CoreAddons ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5TextWidgets ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5XmlGui ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5KIO ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5Config ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5Codecs ${KF5_MIN_VERSION} CONFIG REQUIRED)

find_package(KF5PimTextEdit ${PIMTEXTEDIT_LIB_VERSION} CONFIG REQUIRED)
add_definitions(-DTRANSLATION_DOMAIN=\"libkpimidentities5\")

option(USE_UNITY_CMAKE_SUPPORT "Use UNITY cmake support (speedup compile time)" OFF)

set(COMPILE_WITH_UNITY_CMAKE_SUPPORT OFF)
if (USE_UNITY_CMAKE_SUPPORT)
    set(COMPILE_WITH_UNITY_CMAKE_SUPPORT ON)
endif()

add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x050f00)
add_definitions(-DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x055000)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
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


kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
ki18n_install(po)
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
