include(ECMAddTests)
set(QT_REQUIRED_VERSION "5.2.0")
find_package(Qt5Test ${QT_REQUIRED_VERSION} CONFIG REQUIRED)


ecm_add_tests( identitytest.cpp signaturetest.cpp NAME_PREFIX "kpimidentity-" LINK_LIBRARIES KF5IdentityManagement KF5::PimTextEdit Qt5::Test KF5::ConfigCore   KF5::TextWidgets KF5::XmlGui KF5::IconThemes)
