# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause
cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
set(PIM_VERSION "5.23.40")

project(KIdentityManagement VERSION ${PIM_VERSION})

# ECM setup
set(KF_MIN_VERSION "5.240.0")
set(QT_MIN_VERSION "6.4.0")
find_package(ECM ${KF_MIN_VERSION} CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(GenerateExportHeader)
include(ECMGenerateHeaders)
include(ECMGeneratePriFile)

include(ECMSetupVersion)
include(FeatureSummary)
include(KDEGitCommitHooks)
include(KDEClangFormat)
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h *.c)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

include(ECMQtDeclareLoggingCategory)
include(ECMDeprecationSettings)
include(ECMAddQch)

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

set(KIDENTITYMANAGEMENT_LIB_VERSION ${PIM_VERSION})
set(PIMTEXTEDIT_LIB_VERSION "5.23.40")

ecm_setup_version(PROJECT VARIABLE_PREFIX KIDENTITYMANAGEMENT
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kidentitymanagement_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KPim6IdentityManagementConfigVersion.cmake"
                        SOVERSION 6
)

########### Find packages ###########

find_package(Qt6Network NO_MODULE REQUIRED)
if (QT_MAJOR_VERSION STREQUAL "6")
    find_package(Qt6Core5Compat)
endif()

find_package(KF6CoreAddons ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF6TextWidgets ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF6XmlGui ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF6KIO ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF6Config ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF6Codecs ${KF_MIN_VERSION} CONFIG REQUIRED)

find_package(KPim6TextEdit ${PIMTEXTEDIT_LIB_VERSION} CONFIG REQUIRED)
add_definitions(-DTRANSLATION_DOMAIN=\"libkpimidentities5\")

option(USE_UNITY_CMAKE_SUPPORT "Use UNITY cmake support (speedup compile time)" OFF)

set(COMPILE_WITH_UNITY_CMAKE_SUPPORT OFF)
if (USE_UNITY_CMAKE_SUPPORT)
    set(COMPILE_WITH_UNITY_CMAKE_SUPPORT ON)
endif()

ecm_set_disabled_deprecation_versions(QT 6.4  KF 5.105.0)


########### Targets ###########
add_subdirectory(src)

if(BUILD_TESTING)
    add_subdirectory(autotests)
endif()

########### CMake Config Files ###########
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KPim6IdentityManagement")
configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KPimIdentityManagementConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KPim6IdentityManagementConfig.cmake"
  INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KPim6IdentityManagementConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KPim6IdentityManagementConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KPim6IdentityManagementTargets
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    FILE KPim6IdentityManagementTargets.cmake
    NAMESPACE KPim6::
)

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/kidentitymanagement_version.h
   DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim6/KIdentityManagement
   COMPONENT Devel
)

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KPim6IdentityManagement_QCH
        FILE KPim6IdentityManagementQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KPim6IdentityManagementQchTargets.cmake\")")
endif()

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
ki18n_install(po)
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
