PROJECT( krichtexteditor )
FIND_PACKAGE(KDE4 REQUIRED)
INCLUDE_DIRECTORIES( ${KDE4_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/.. )

SET(krichtexteditorSources
  main.cpp
  krichtexteditor.cpp
  ../kmarkupdirector.cpp
  ../kmarkupdirector_p.cpp
  ../ktexthtmlbuilder.cpp
  ../kplaintextmarkupbuilder.cpp
  ../mediawikimarkupbuilder.cpp
  ../bbcodebuilder.cpp
)

KDE4_ADD_EXECUTABLE(krichtexteditor ${krichtexteditorSources} )
TARGET_LINK_LIBRARIES(krichtexteditor ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} )

install(TARGETS krichtexteditor ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
  krichtexteditorui.rc
  DESTINATION  ${DATA_INSTALL_DIR}/krichtexteditor)



