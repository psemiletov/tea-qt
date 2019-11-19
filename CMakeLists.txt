cmake_minimum_required(VERSION 3.0)
set (QT_MIN_VERSION "5.4.0")

include_directories(${Qt5Widgets_INCLUDE_DIRS})
add_definitions(${Qt5Widgets_DEFINITIONS})
add_definitions(${Qt5Widgets_COMPILE_DEFINITIONS})


set(PROJECT "tea-qt")
project ($PROJECT VERSION 49.0.0)

add_definitions(-DVERSION_NUMBER="\\"${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}\\"")


if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
   add_definitions(-DQ_OS_LINUX)
   add_definitions(-DQ_OS_UNIX)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

message("CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")


find_package(PkgConfig REQUIRED)
pkg_check_modules(hunspell hunspell)
if(hunspell_FOUND)
  add_definitions(-DHUNSPELL_ENABLE)
  message("+ hunspell support")
endif()


find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED Core Widgets Network)


if(DEFINED USE_QML)
message("QML IS SUPPORTED")
find_package(Qt5 ${QT_MIN_VERSION} CONFIG OPTIONAL_COMPONENTS Qml Quick)
add_definitions(-DUSE_QML_STUFF)
endif(DEFINED USE_QML)


if(BUILD_TESTING)
  find_package(Qt5Test ${QT_MIN_VERSION} CONFIG REQUIRED)
endif()


qt5_add_resources(QT_RESOURCES resources.qrc)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")


file(GLOB tea_SRCS "*.c" "*.cpp")
file(GLOB tea_HEADERS "*.h")


set(${PROJECT}_ICONPNG32
    ./icons/32/${PROJECT}.png
)


set(${PROJECT}_ICONPNG48
    ./icons/48/${PROJECT}.png
)


set(${PROJECT}_ICONPNG64
    ./icons/64/${PROJECT}.png
)


set(${PROJECT}_ICONPNG128
    ./icons/128/${PROJECT}.png
)

set(${PROJECT}_ICONSVG
    ./icons/svg/${PROJECT}.svg
)

set(${PROJECT}_DESKTOP
    ./desktop/${PROJECT}.desktop
)


#add_custom_target(dist
#COMMAND git archive --prefix=${PROJECT}-${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}/ master | bzip2 >${PROJECT}-${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.tar.bz2
#WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
#)

add_custom_target(dist
COMMAND git archive --format=tar --prefix=${PROJECT}-${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}/ HEAD | gzip >${PROJECT}-${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.tar.gz
WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)



add_executable(tea ${tea_SRCS} ${QT_RESOURCES})


find_package(Qt5PrintSupport)
if (Qt5PrintSupport_FOUND)
   Message ("+ printer support")
   add_definitions(-DPRINTER_ENABLE)
   target_link_libraries(tea Qt5::PrintSupport)
endif()


if (EXISTS "/usr/include/linux/joystick.h")
   message("+JOYSTICK_SUPPORTED")
   add_definitions(-DJOYSTICK_SUPPORTED)
endif()


find_path(ASPELL_INLCUDES aspell.h
         "/usr/include/"
         "/usr/local/"
        )

if(ASPELL_INLCUDES)
  add_definitions(-DASPELL_ENABLE)
  target_link_libraries(tea libaspell.so)
  message("+ aspell support")
endif()


find_package(PkgConfig)
pkg_check_modules(popplerqt5 poppler-qt5)
if(popplerqt5_FOUND)
  add_definitions(-DPOPPLER_ENABLE)
  target_link_libraries(tea ${popplerqt5_LIBRARIES})
  include_directories(${popplerqt5_INCLUDE_DIRS})
  message("+ poppler support")
endif()


find_package(PkgConfig)
pkg_check_modules(ddjvuapi ddjvuapi)
if(ddjvuapi_FOUND)
  add_definitions(-DDJVU_ENABLE)
  target_link_libraries(tea ${ddjvuapi_LIBRARIES})
  include_directories(${ddjvuapi_INCLUDE_DIRS})

  message("+ djvuapi support")
endif()


if(UNIX OR MINGW)
    find_package(ZLIB REQUIRED)
else(UNIX OR MINGW)
    set(ZLIB_INCLUDE_DIRS "${QT_ROOT}/src/3rdparty/zlib" CACHE STRING "Path to ZLIB headers of Qt")
    set(ZLIB_LIBRARIES "")
    if(NOT EXISTS "${ZLIB_INCLUDE_DIRS}/zlib.h")
        message("Please specify a valid zlib include dir")
    endif(NOT EXISTS "${ZLIB_INCLUDE_DIRS}/zlib.h")
endif(UNIX OR MINGW)


if (ZLIB_FOUND)
    include_directories(${ZLIB_INCLUDE_DIRS} )
    target_link_libraries(tea ${ZLIB_LIBRARIES} )
endif( ZLIB_FOUND )

if(hunspell_FOUND)
  include_directories(${hunspell_INCLUDE_DIRS} )
  target_link_libraries(tea ${hunspell_LIBRARIES} )
endif()

target_link_libraries(tea Qt5::Widgets Qt5::Core Qt5::Network)

if(DEFINED USE_QML)
target_link_libraries(tea Qt5::Qml Qt5::Quick)
endif()


install (TARGETS tea DESTINATION bin)
install (FILES ${tea_ICONSVG} DESTINATION share/icons/hicolor/scalable/apps)
install (FILES ${tea_ICONPNG32} DESTINATION share/icons/hicolor/32x32/apps)
install (FILES ${tea_ICONPNG48} DESTINATION share/icons/hicolor/48x48/apps)
install (FILES ${tea_ICONPNG64} DESTINATION share/icons/hicolor/64x64/apps)
install (FILES ${tea_ICONPNG128} DESTINATION share/icons/hicolor/128x128/apps)

install (FILES ${tea_DESKTOP} DESTINATION share/applications)

