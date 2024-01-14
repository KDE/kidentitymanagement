# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause
cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
set(PIM_VERSION "6.0.40")

project(KIdentityManagement VERSION ${PIM_VERSION})

# ECM setup
set(KF_MIN_VERSION "5.246.0")
set(QT_REQUIRED_VERSION "6.6.0")
find_package(ECM ${KF_MIN_VERSION} CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(GenerateExportHeader)
include(ECMGenerateHeaders)

include(ECMSetupVersion)
include(FeatureSummary)
include(KDEGitCommitHooks)
include(KDEClangFormat)
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h *.c)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

include(ECMQtDeclareLoggingCategory)
include(ECMDeprecationSettings)
include(ECMAddQch)
include(ECMQmlModule)

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

option(BUILD_QUICK_SUPPORT "Build quick support. Needed for merkuro project" ON)

set(KIDENTITYMANAGEMENT_LIB_VERSION ${PIM_VERSION})
set(PIMTEXTEDIT_LIB_VERSION "6.0.40")
set(KTEXTADDONS_MIN_VERSION "1.5.3")

########### Find packages ###########

find_package(Qt6Network NO_MODULE REQUIRED)

if (BUILD_QUICK_SUPPORT)
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
if (BUILD_QUICK_SUPPORT)
   find_package(KF6KirigamiAddons 0.11 REQUIRED)
   set_package_properties(KF6KirigamiAddons PROPERTIES
       TYPE REQUIRED
       PURPOSE "Addons for the Kirigami application UI framework"
   )
endif()
add_definitions(-DTRANSLATION_DOMAIN=\"libkpimidentities6\")

option(USE_UNITY_CMAKE_SUPPORT "Use UNITY cmake support (speedup compile time)" OFF)

set(COMPILE_WITH_UNITY_CMAKE_SUPPORT OFF)
if (USE_UNITY_CMAKE_SUPPORT)
    set(COMPILE_WITH_UNITY_CMAKE_SUPPORT ON)
endif()

add_definitions(-DQT_NO_CONTEXTLESS_CONNECT)
ecm_set_disabled_deprecation_versions(QT 6.6.0 KF 5.248.0)


########### Targets ###########
add_subdirectory(src)

if(BUILD_TESTING)
    add_subdirectory(autotests)
endif()

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
ki18n_install(po)
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
