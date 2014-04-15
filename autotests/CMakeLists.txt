include(ECMMarkAsTest)

set(QT_REQUIRED_VERSION "5.2.0")
find_package(Qt5Test ${QT_REQUIRED_VERSION} CONFIG REQUIRED)

########### next target ###############

set(identitytest_SRCS identitytest.cpp)

add_executable(identitytest ${identitytest_SRCS})
add_test(identitytest identitytest)
ecm_mark_as_test(identitytest)
target_link_libraries(identitytest KF5::PimIdentities Qt5::Test)

########### next target ###############

set(signaturetest_SRCS signaturetest.cpp)

add_executable(signaturetest ${signaturetest_SRCS})
add_test(signaturetest signaturetest)
ecm_mark_as_test(signaturetest)
target_link_libraries(signaturetest KF5PimIdentities KF5::PimTextEdit Qt5::Test)
