
find_package(KF5 NO_MODULE REQUIRED COMPONENTS CoreAddons KIO Sonnet)
set(kpimidentities_LIB_SRCS identitymanager.cpp identity.cpp signature.cpp signatureconfigurator.cpp identitycombo.cpp)

add_subdirectory( tests )

qt4_add_dbus_adaptor(kpimidentities_LIB_SRCS org.kde.pim.IdentityManager.xml identitymanager.h KPIMIdentities::IdentityManager )

kde4_add_library(kpimidentities ${LIBRARY_TYPE} ${kpimidentities_LIB_SRCS})

target_link_libraries(kpimidentities KF5::CoreAddons KF5::KIOCore KF5::SonnetCore kpimutils kpimtextedit )

set_target_properties(kpimidentities PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kpimidentities EXPORT kdepimlibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES org.kde.pim.IdentityManager.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})
install(FILES
        identity.h
        signature.h
        signatureconfigurator.h
        identitymanager.h
        identitycombo.h
        kpimidentities_export.h
        DESTINATION ${INCLUDE_INSTALL_DIR}/kpimidentities COMPONENT Devel)

