set (identitytest_SRCS identitytest.cpp )
kde4_add_unit_test (identitytest TESTNAME identitytest ${identitytest_SRCS})

target_link_libraries ( identitytest
                        ${QT_QTTEST_LIBRARY}
                        ${QT_QTCORE_LIBRARY}
                        ${KDE4_KDECORE_LIBS}
                        kpimidentities
                      )

set (signaturetest_SRCS signaturetest.cpp )
kde4_add_unit_test (signaturetest TESTNAME signaturetest ${signaturetest_SRCS})

target_link_libraries ( signaturetest
                        ${QT_QTTEST_LIBRARY}
                        ${QT_QTCORE_LIBRARY}
                        ${KDE4_KDECORE_LIBS}
                        kpimidentities
                        kpimtextedit
                      )
