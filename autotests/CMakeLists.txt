include(ECMAddTests)
find_package(Qt5Test ${QT_REQUIRED_VERSION} CONFIG REQUIRED)
add_definitions( -DKIDENTITYMANAGER_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/")

ecm_add_tests( identitytest.cpp signaturetest.cpp NAME_PREFIX "kpimidentity-" LINK_LIBRARIES KF5IdentityManagement
    KF5::PimTextEdit Qt::Test KF5::ConfigCore   KF5::TextWidgets KF5::XmlGui KF5::CoreAddons)
