cmake_minimum_required(VERSION 3.0)
set (QT_MIN_VERSION "5.4.0")

include_directories(${Qt5Widgets_INCLUDE_DIRS})
add_definitions(${Qt5Widgets_DEFINITIONS})
add_definitions(${Qt5Widgets_COMPILE_DEFINITIONS})




set(PROJECT "tea")
project ($PROJECT VERSION 45.0.3)

add_definitions(-DVERSION_NUMBER="\\"${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}\\"")
#add_definitions(-DQUAZIP_STATIC)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
   add_definitions(-DQ_OS_LINUX)
   add_definitions(-DQ_OS_UNIX)
endif()   

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

message("CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")

#add_subdirectory (quazip)


find_package(PkgConfig REQUIRED)
#pkg_check_modules(hunspell REQUIRED hunspell)
pkg_check_modules(hunspell hunspell)
if(hunspell_FOUND)
  add_definitions(-DHUNSPELL_ENABLE)
  add_definitions(-DSPELLCHECK_ENABLE)
  message("+ hunspell support")
endif()


#find_library(ASPELL_LIB
#           NAMES libaspell
#           PATHS /usr/lib /usr/local/lib /usr/lib64 /usr/local/lib64
#           )
 
 

#include_directories (${aspell_INCLUDE_DIRS})


find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED Core Widgets Network)

if(BUILD_TESTING)
  find_package(Qt5Test ${QT_MIN_VERSION} CONFIG REQUIRED)
endif()


qt5_add_resources(QT_RESOURCES rvln.qrc)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")


file(GLOB tea_SRCS "*.c" "*.cpp")
file(GLOB tea_HEADERS "*.h")


set(${PROJECT}_ICONPNG
    ./icons/${PROJECT}.png
)

set(${PROJECT}_ICONSVG
    ./icons/${PROJECT}.svg
)

set(${PROJECT}_DESKTOP
    ./desktop/${PROJECT}.desktop
)


add_custom_target(dist
COMMAND git archive --prefix=${PROJECT}-${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}/ master | bzip2 >${PROJECT}-${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.tar.bz2
WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)


add_executable(${PROJECT} ${tea_SRCS} ${QT_RESOURCES})

find_package(Qt5PrintSupport)
if (Qt5PrintSupport_FOUND)
   Message ("+ printer support")
   add_definitions(-DPRINTER_ENABLE)
   target_link_libraries(${PROJECT} Qt5::PrintSupport)
endif()


#find_package( ZLIB REQUIRED )
#if ( ZLIB_FOUND )
#    include_directories(${ZLIB_INCLUDE_DIRS} )
    #target_link_libraries(${PROJECT} ${ZLIB_LIBRARIES} )
#endif( ZLIB_FOUND )


#find_package(ASPELL)
#if (ASPELL_FOUND)
  #message("+ Aspell support")
  #add_definitions(-DASPELL_ENABLE)
  #add_definitions(-DSPELLCHECK_ENABLE)
  
  #include_directories(${aspell_INCLUDE_DIRS} )
  #target_link_libraries(${PROJECT} ${aspell_LIBRARIES} )
  #add_definitions(${ASPELL_DEFINITIONS})
#endif()


find_path(ASPELL_INLCUDES aspell.h
         "/usr/include/"
         "/usr/local/"
         
        )      
            
if(ASPELL_INLCUDES)
  add_definitions(-DASPELL_ENABLE)
  add_definitions(-DSPELLCHECK_ENABLE)
  target_link_libraries(${PROJECT} libaspell.so)
  message("+ aspell support")
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
    target_link_libraries(${PROJECT} ${ZLIB_LIBRARIES} )
endif( ZLIB_FOUND )

if(hunspell_FOUND)
  include_directories(${hunspell_INCLUDE_DIRS} )
  target_link_libraries(${PROJECT} ${hunspell_LIBRARIES} )
endif()

target_link_libraries(${PROJECT} Qt5::Widgets Qt5::Core Qt5::Network)

install (TARGETS ${PROJECT} DESTINATION bin)
install (FILES ${upsm_ICONSVG} DESTINATION share/icons/hicolor/scalable/apps)
install (FILES ${upsm_ICONPNG} DESTINATION share/icons/hicolor/128x128/apps)
install (FILES ${upsm_DESKTOP} DESTINATION share/applications)
