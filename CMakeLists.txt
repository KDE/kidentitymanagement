# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause
cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
set(PIM_VERSION "6.3.40")

project(KIdentityManagement VERSION ${PIM_VERSION})

# ECM setup
set(KF_MIN_VERSION "6.8.0")
set(QT_REQUIRED_VERSION "6.7.0")
find_package(ECM ${KF_MIN_VERSION} CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(GenerateExportHeader)
include(ECMGenerateHeaders)

include(ECMSetupVersion)
include(KDEGitCommitHooks)
include(KDEClangFormat)
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES src/*.cpp src/*.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

include(ECMQtDeclareLoggingCategory)
include(ECMDeprecationSettings)
include(ECMFeatureSummary)
include(ECMAddQch)
include(ECMQmlModule)
include(ECMCheckOutboundLicense)
option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

option(BUILD_QUICK_SUPPORT "Build quick support. Needed for merkuro project" ON)

set(KIDENTITYMANAGEMENT_LIB_VERSION ${PIM_VERSION})
set(PIMTEXTEDIT_LIB_VERSION "6.3.40")
set(KTEXTADDONS_MIN_VERSION "1.5.4")

########### Find packages ###########

find_package(Qt6Network NO_MODULE REQUIRED)
find_package(Qt6DBus ${QT_REQUIRED_VERSION} CONFIG REQUIRED)

if(BUILD_QUICK_SUPPORT)
   find_package(Qt6Quick NO_MODULE REQUIRED)
endif()

find_package(KF6CoreAddons ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF6XmlGui ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF6KIO ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF6Config ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF6Codecs ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF6I18n   ${KF_MIN_VERSION} CONFIG REQUIRED)

find_package(KF6TextCustomEditor ${KTEXTADDONS_MIN_VERSION} CONFIG REQUIRED)
find_package(KPim6TextEdit ${PIMTEXTEDIT_LIB_VERSION} CONFIG REQUIRED)

if(BUILD_TESTING)
    find_package(Qt6Test ${QT_REQUIRED_VERSION} CONFIG REQUIRED)
endif()

if(BUILD_QUICK_SUPPORT)
   find_package(KF6KirigamiAddons 1.0 REQUIRED)
   set_package_properties(KF6KirigamiAddons PROPERTIES
       TYPE REQUIRED
       PURPOSE "Addons for the Kirigami application UI framework"
  )
endif()
add_definitions(-DTRANSLATION_DOMAIN=\"libkpimidentities6\")

option(USE_UNITY_CMAKE_SUPPORT "Use UNITY cmake support (speedup compile time)" OFF)

set(COMPILE_WITH_UNITY_CMAKE_SUPPORT OFF)
if(USE_UNITY_CMAKE_SUPPORT)
    set(COMPILE_WITH_UNITY_CMAKE_SUPPORT ON)
endif()

add_definitions(-DQT_NO_CONTEXTLESS_CONNECT)
# -fhardened increase compilation time and produce a lot of warning
option(OPTION_DISABLE_HARDENED_OPTIMIZATION "Disable -fhardened" OFF)
if(OPTION_DISABLE_HARDENED_OPTIMIZATION)
   string(REPLACE "-fhardened" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
   string(REPLACE "-fhardened" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()
ecm_set_disabled_deprecation_versions(QT 6.8.1 KF 6.10.0)


########### Targets ###########
add_subdirectory(src)

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
ki18n_install(po)
ecm_feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
file(GLOB_RECURSE ALL_SOURCE_FILES src/*.cpp src/*.h)
ecm_check_outbound_license(LICENSES GPL-2.0-only  FILES ${ALL_SOURCE_FILES})
