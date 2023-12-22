# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause
cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
set(PIM_VERSION "5.24.5")

project(KIdentityManagement VERSION ${PIM_VERSION})

# ECM setup
set(KF_MIN_VERSION "5.105.0")
set(QT_REQUIRED_VERSION "5.15.2")
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

if (QT_MAJOR_VERSION STREQUAL "6")
    set(QT_MIN_VERSION "6.4.0")
    set(KF_MIN_VERSION "5.240.0")
    set(KF_MAJOR_VERSION "6")
else()
    set(KF_MAJOR_VERSION "5")
endif()

option(BUILD_WITH_COMPAT_LIBS "Generate compat cmake file (Set at false will allow to co-install lib)" ON)

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

set(KIDENTITYMANAGEMENT_LIB_VERSION ${PIM_VERSION})
set(PIMTEXTEDIT_LIB_VERSION "5.24.4")

ecm_setup_version(PROJECT VARIABLE_PREFIX KIDENTITYMANAGEMENT
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kidentitymanagement_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KPim${KF_MAJOR_VERSION}IdentityManagementConfigVersion.cmake"
                        SOVERSION 5
)

########### Find packages ###########

find_package(Qt${QT_MAJOR_VERSION}Network NO_MODULE REQUIRED)
if (QT_MAJOR_VERSION STREQUAL "6")
    find_package(Qt6Core5Compat)
endif()

find_package(KF${KF_MAJOR_VERSION}CoreAddons ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF${KF_MAJOR_VERSION}TextWidgets ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF${KF_MAJOR_VERSION}XmlGui ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF${KF_MAJOR_VERSION}KIO ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF${KF_MAJOR_VERSION}Config ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF${KF_MAJOR_VERSION}Codecs ${KF_MIN_VERSION} CONFIG REQUIRED)

find_package(KPim${KF_MAJOR_VERSION}TextEdit ${PIMTEXTEDIT_LIB_VERSION} CONFIG REQUIRED)
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
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KPim${KF_MAJOR_VERSION}IdentityManagement")
set(KIDENTIFY_KF5_COMPAT FALSE)
configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KPimIdentityManagementConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KPim${KF_MAJOR_VERSION}IdentityManagementConfig.cmake"
  INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KPim${KF_MAJOR_VERSION}IdentityManagementConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KPim${KF_MAJOR_VERSION}IdentityManagementConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KPim${KF_MAJOR_VERSION}IdentityManagementTargets
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    FILE KPim${KF_MAJOR_VERSION}IdentityManagementTargets.cmake
    NAMESPACE KPim${KF_MAJOR_VERSION}::
)

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/kidentitymanagement_version.h
   DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim${KF_MAJOR_VERSION}/KIdentityManagement
   COMPONENT Devel
)

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KPim${KF_MAJOR_VERSION}IdentityManagement_QCH
        FILE KPim${KF_MAJOR_VERSION}IdentityManagementQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KPim${KF_MAJOR_VERSION}IdentityManagementQchTargets.cmake\")")
endif()

if (BUILD_WITH_COMPAT_LIBS)
if (QT_MAJOR_VERSION STREQUAL "5")
##
# TODO: Backwards compatiblity. Remove in next major version
##
set(CMAKECONFIG_INSTALL_DIR_KF5 "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5IdentityManagement")
set(KIDENTIFY_KF5_COMPAT TRUE)
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/KPimIdentityManagementConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/KF5IdentityManagementConfig.cmake"
    INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR_KF5}
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/KF5IdentityManagementConfig.cmake"
    DESTINATION "${CMAKECONFIG_INSTALL_DIR_KF5}"
    COMPONENT Devel
)
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/KPim${KF_MAJOR_VERSION}IdentityManagementConfigVersion.cmake"
    RENAME "KF5IdentityManagementConfigVersion.cmake"
    DESTINATION "${CMAKECONFIG_INSTALL_DIR_KF5}"
    COMPONENT Devel
)
install(EXPORT KPim${KF_MAJOR_VERSION}IdentityManagementTargets
    DESTINATION "${CMAKECONFIG_INSTALL_DIR_KF5}"
    FILE KPim${KF_MAJOR_VERSION}IdentityManagementTargets.cmake
    NAMESPACE KF5::
)

if (BUILD_QCH)
    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/KPim${KF_MAJOR_VERSION}IdentityManagementQchTargets.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR_KF5}"
        COMPONENT Devel
    )
endif()


endif()
endif()

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
ki18n_install(po)
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
