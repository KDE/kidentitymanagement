# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause
include(ECMAddTests)
find_package(Qt${QT_MAJOR_VERSION}Test ${QT_REQUIRED_VERSION} CONFIG REQUIRED)

macro(add_identity_test _source)
    get_filename_component(_name ${_source} NAME_WE)
    ecm_add_test(${_source} ${_name}.h
        TEST_NAME ${_name}
        NAME_PREFIX "kpimidentity-"
        LINK_LIBRARIES KPim${KF_MAJOR_VERSION}IdentityManagementWidgets
        KPim${KF_MAJOR_VERSION}::PimTextEdit Qt::Test KF${KF_MAJOR_VERSION}::ConfigCore KF${KF_MAJOR_VERSION}::TextWidgets KF${KF_MAJOR_VERSION}::XmlGui KF${KF_MAJOR_VERSION}::CoreAddons
    )
endmacro ()

add_identity_test(identitytest.cpp)
add_identity_test(signaturetest.cpp)

target_compile_definitions(signaturetest PRIVATE
    SIGNATURETEST_DATA_FILE="${CMAKE_CURRENT_SOURCE_DIR}/signaturetest.cpp"
)
