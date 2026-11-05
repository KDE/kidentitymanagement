# SPDX-FileCopyrightText: 2026 Carl Schwan <carl@carlschwan.eu>
# SPDX-License-Identifier: BSD-2-Clause

ecm_add_test(identityeditorbackendtest.cpp
    TEST_NAME identityeditorbackendtest
    NAME_PREFIX "kpimidentityquick-"
    LINK_LIBRARIES identitymanagentprivate Qt::Test
)
