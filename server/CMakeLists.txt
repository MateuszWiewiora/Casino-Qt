cmake_minimum_required(VERSION 3.5)

project(casino-server VERSION 0.1 LANGUAGES CXX)

set(PROJECT_SOURCES
    serverwindow.cpp
    serverwindow.h
    serverwindow.ui
)

add_library(casino-server STATIC
    ${PROJECT_SOURCES}
)

target_link_libraries(casino-server PUBLIC
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
)

target_include_directories(casino-server PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.server)
endif()
set_target_properties(casino-server PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS casino-server
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(casino-server)
endif()

