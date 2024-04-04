# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause
include(ECMAddTests)

macro(add_identitycore_test _source)
    get_filename_component(_name ${_source} NAME_WE)
    ecm_add_test(${_source} ${_name}.h
        TEST_NAME ${_name}
        NAME_PREFIX "kpimidentitycore-"
	LINK_LIBRARIES KPim6::IdentityManagementCore
        KPim6::PimTextEdit Qt::Test KF6::ConfigCore KF6::XmlGui KF6::CoreAddons
    )
endmacro ()

add_identitycore_test(identitytest.cpp)
add_identitycore_test(signaturetest.cpp)
add_identitycore_test(identitymodeltest.cpp)

target_compile_definitions(signaturetest PRIVATE
    SIGNATURETEST_DATA_FILE="${CMAKE_CURRENT_SOURCE_DIR}/signaturetest.cpp"
)
