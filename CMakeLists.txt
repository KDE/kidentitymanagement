########### next target ###############

set(kpimidentities_LIB_SRCS identitymanager.cpp identity.cpp identitycombo.cpp)

kde4_automoc(${kpimidentities_LIB_SRCS})

qt4_add_dbus_adaptor(kpimidentities_LIB_SRCS org.kde.pim.IdentityManager.xml identitymanager.h KPIMIdentities::IdentityManager )

kde4_add_library(kpimidentities SHARED ${kpimidentities_LIB_SRCS})

target_link_libraries(kpimidentities ${QT_AND_KDECORE_LIBS}  ${KDE4_KDEUI_LIBS} kpimutils ${KDE4_KUTILS_LIBS}) 

set_target_properties(kpimidentities PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kpimidentities  DESTINATION ${LIB_INSTALL_DIR})

install(FILES org.kde.pim.IdentityManager.xml DESTINATION ${DBUS_INTERFACES_DIR})
install(FILES
        identity.h
        identitymanager.h
        identitycombo.h
        libkpimidentities_export.h
        DESTINATION ${INCLUDE_INSTALL_DIR}/kpimidentities )

