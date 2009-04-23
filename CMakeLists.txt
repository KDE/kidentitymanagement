
set(kpimidentities_LIB_SRCS identitymanager.cpp identity.cpp signature.cpp signatureconfigurator.cpp identitycombo.cpp)


qt4_add_dbus_adaptor(kpimidentities_LIB_SRCS org.kde.pim.IdentityManager.xml identitymanager.h KPIMIdentities::IdentityManager )

kde4_add_library(kpimidentities SHARED ${kpimidentities_LIB_SRCS})

target_link_libraries(kpimidentities ${QT_AND_KDECORE_LIBS}  kpimutils ${KDE4_KIO_LIBS} )

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

