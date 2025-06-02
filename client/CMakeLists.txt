cmake_minimum_required(VERSION 3.5)

project(casino-client VERSION 0.1 LANGUAGES CXX)

set(PROJECT_SOURCES
    mainwindow.h mainwindow.cpp mainwindow.ui
    loginwindow.h loginwindow.cpp loginwindow.ui
    clickablelabel.h clickablelabel.cpp
    minigame1.h minigame1.cpp
    minigame2.h minigame2.cpp
)

add_library(casino-client STATIC
    ${PROJECT_SOURCES}

)

target_link_libraries(casino-client PUBLIC
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
)

target_include_directories(casino-client PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.Casino-Qt-Client)
endif()

set_target_properties(casino-client PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS casino-client
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(casino-client)
endif()
