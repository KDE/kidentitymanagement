include(ECMAddTests)
find_package(Qt5Test ${QT_REQUIRED_VERSION} CONFIG REQUIRED)

ecm_add_tests( identitytest.cpp signaturetest.cpp NAME_PREFIX "kpimidentity-" LINK_LIBRARIES KF5IdentityManagement
    KF5::PimTextEdit Qt::Test KF5::ConfigCore   KF5::TextWidgets KF5::XmlGui KF5::CoreAddons)

target_compile_definitions(signaturetest PRIVATE
    SIGNATURETEST_DATA_FILE="${CMAKE_CURRENT_SOURCE_DIR}/signaturetest.cpp"
)
