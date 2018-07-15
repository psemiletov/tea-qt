cmake_minimum_required(VERSION 3.0)
set (QT_MIN_VERSION "5.4.0")

include_directories(${Qt5Widgets_INCLUDE_DIRS})
add_definitions(${Qt5Widgets_DEFINITIONS})

set(PROJECT "upsm")
project ($PROJECT VERSION 2.0.1)

add_definitions(-DVERSION="\\"2.0.99-2.1\\"")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)


find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED Core Widgets)

if(BUILD_TESTING)
  find_package(Qt5Test ${QT_MIN_VERSION} CONFIG REQUIRED)
endif()


qt5_add_resources(QT_RESOURCES upsm.qrc)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

# Populate a CMake variable with the sources
set(upsm_SRCS
    mainwindow.cpp
    main.cpp
)

set(upsm_ICONPNG
    ./icons/upsm.png
)

set(upsm_ICONSVG
    ./icons/upsm.svg
)


set(upsm_DESKTOP
    ./desktop/upsm.desktop
)


add_custom_target(dist
COMMAND git archive --prefix=${PROJECT}-${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}/ master | bzip2 >${PROJECT}-${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.tar.bz2
WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)




add_executable(${PROJECT} ${upsm_SRCS} ${QT_RESOURCES})

find_package( ZLIB REQUIRED )
if ( ZLIB_FOUND )
    include_directories( ${ZLIB_INCLUDE_DIRS} )
    target_link_libraries( YourProject ${ZLIB_LIBRARIES} )
endif( ZLIB_FOUND )

target_link_libraries(${PROJECT} Qt5::Widgets Qt5::Core Qt5::Network)

install (TARGETS ${PROJECT} DESTINATION bin)
install (FILES ${upsm_ICONSVG} DESTINATION share/icons/hicolor/scalable/apps)
install (FILES ${upsm_ICONPNG} DESTINATION share/icons/hicolor/128x128/apps)
install (FILES ${upsm_DESKTOP} DESTINATION share/applications)
