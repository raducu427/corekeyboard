cmake_minimum_required( VERSION 3.16 )
project( corekeyboard )

set( PROJECT_VERSION 5.0.0 )
set( PROJECT_VERSION_MAJOR 5 )
set( PROJECT_VERSION_MINOR 0 )
set( PROJECT_VERSION_PATCH 0 )

set( PROJECT_VERSION_MAJOR_MINOR ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR} )
add_compile_definitions(VERSION_TEXT="${PROJECT_VERSION}")

set( APP_ID "cc.cubocore.CoreKeyboard" )
add_compile_definitions( APP_ID=\"${APP_ID}\" )

set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_INCLUDE_CURRENT_DIR ON )
set( CMAKE_BUILD_TYPE Release )
set( CMAKE_AUTOMOC ON )
set( CMAKE_AUTORCC ON )
set( CMAKE_AUTOUIC ON )

add_definitions ( -Wall )
if ( CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT )
    set( CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "Location for installing the project" FORCE )
endif()

find_package( Qt6Core REQUIRED )
find_package( Qt6Gui REQUIRED )
find_package( Qt6Widgets REQUIRED )
find_package( Qt6Network REQUIRED )

set( SOURCES
    src/main.cpp
    src/fixx11h.h
    src/trayicon.h
    src/settings.cpp
    src/settings.h
    src/corekeyboard.cpp
    src/corekeyboard.h
    src/suggestions.cpp
    src/suggestions.h
)

set ( RSCS
    resources.qrc
)

add_executable( corekeyboard ${SOURCES} ${RSCS} )
target_link_libraries ( corekeyboard  Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network cprime-widgets cprime-core X11 Xtst )

install( TARGETS corekeyboard DESTINATION bin )
install( FILES ${APP_ID}.desktop DESTINATION share/applications )
install( FILES ${APP_ID}.svg DESTINATION share/icons/hicolor/scalable/apps/ )
install( FILES ${APP_ID}.metainfo.xml DESTINATION share/metainfo/ )

