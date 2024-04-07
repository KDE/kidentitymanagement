# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause
include(ECMAddTests)

macro(add_identitywidget_test _source)
    get_filename_component(_name ${_source} NAME_WE)
    ecm_add_test(${_source} ${_name}.h
        TEST_NAME ${_name}
        NAME_PREFIX "kpimidentitywidgets-"
	LINK_LIBRARIES KPim6::IdentityManagementWidgets
        KPim6::PimTextEdit Qt::Test KF6::ConfigCore KF6::XmlGui KF6::CoreAddons
    )
endmacro ()

