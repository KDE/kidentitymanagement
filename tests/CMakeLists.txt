set (identitytest_SRCS identitytest.cpp )
kde4_add_unit_test (identitytest TESTNAME identitytest ${identitytest_SRCS})

target_link_libraries ( identitytest
                        ${QT_QTTEST_LIBRARY}
                        ${QT_QTCORE_LIBRARY}
                        ${KDE4_KDECORE_LIBS}
                        kpimidentities
                      )
