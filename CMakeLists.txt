#cmake_minimum_required(VERSION 3.0)
cmake_minimum_required(VERSION 3.21.1)

set (QT_MIN_VERSION "5.4.0")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

enable_language(CXX)
enable_language(C)

find_package(Qt6 COMPONENTS Core Widgets)
if (NOT Qt6_FOUND)
    find_package(Qt5 5.15 REQUIRED COMPONENTS Core Widgets)
endif()


if (Qt6_FOUND)
message("+ Qt6 found")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#find_package(Qt6 COMPONENTS Core5Compat REQUIRED)
endif()


#qt_standard_project_setup()
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)


set(PROJECT "tea-qt")
project ($PROJECT VERSION 63.0.0 LANGUAGES CXX C)
qt_add_resources(QT_RESOURCES resources.qrc)

add_definitions(-DVERSION_NUMBER="\\"${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}\\"")
#add_definitions(-DNOCRYPT=1)
#add_definitions(-DNOUNCRYPT=1)
#add_definitions(-DQUAZIP_STATIC=1)

option(USE_NUSPELL "Use Nuspell" OFF)
option(USE_ASPELL "Use Aspell" OFF)
option(USE_HUNSPELL "Use Hunspell" ON)
option(USE_PRINTER "Use printer support" OFF)
option(USE_PDF "Use libpoppler" OFF)
option(USE_DJVU "Use djvu support" OFF)
option(USE_MAC "Build for Mac" OFF)
option(USE_SPEECH "Use Speech Dispatcher" OFF)


if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
   add_definitions(-DQ_OS_LINUX)
   add_definitions(-DQ_OS_UNIX)
endif()


message("CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${QtWidgets_EXECUTABLE_COMPILE_FLAGS}")

file(GLOB tea_SRCS "src/*.c" "src/*.cpp")
file(GLOB tea_HEADERS "src/*.h" "src/.*hpp")


set(tea_ICONPNG32
    ./icons/32/tea.png
)


set(tea_ICONPNG48
    ./icons/48/tea.png
)


set(tea_ICONPNG64
    ./icons/64/tea.png
)


set(tea_ICONPNG128
    ./icons/128/tea.png
)

set(tea_ICONSVG
    ./icons/svg/tea.svg
)

set(tea_DESKTOP
    ./desktop/tea.desktop
)

add_custom_target(dist
COMMAND git archive --format=tar --prefix=${PROJECT}-${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}/ HEAD | gzip >${PROJECT}-${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.tar.gz
WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)


add_executable(tea ${tea_SRCS} ${QT_RESOURCES})

#if (USE_MAC)
#set_target_properties(tea PROPERTIES MACOSX_BUNDLE TRUE)
#endif()

set_target_properties(tea PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)

find_package(PkgConfig REQUIRED)

if (Qt6_FOUND)
if(USE_NUSPELL)
pkg_check_modules(nuspell QUIET nuspell)
if(nuspell_FOUND)
  add_definitions(-DNUSPELL_ENABLE)
  target_link_libraries(tea ${nuspell_LIBRARIES})
  include_directories(${nuspell_INCLUDE_DIRS})
  message("+ nuspell support")
endif()

endif()
endif()
#end of Qt6/Nuspell


if(USE_HUNSPELL)
pkg_check_modules(hunspell QUIET hunspell)
if(hunspell_FOUND)
  add_definitions(-DHUNSPELL_ENABLE)
  message("+ hunspell support")
endif()
endif()


if(USE_SPEECH)
pkg_check_modules(SPEECH REQUIRED speech-dispatcher>=0.0.1)
if (SPEECH_FOUND)
   Message ("+ speech support")
   add_definitions(-DSPEECH_ENABLE)
   target_include_directories(tea PUBLIC ${SPEECH_INCLUDE_DIRS})
   target_link_libraries(tea ${SPEECH_LIBRARIES})
endif()
endif()



if(USE_PRINTER)
find_package(QtPrintSupport)
if (Qt6_FOUND)
   Message ("+ printer support")
   add_definitions(-DPRINTER_ENABLE)
   target_link_libraries(tea Qt::PrintSupport)
endif()
endif()


if (EXISTS "/usr/include/linux/joystick.h")
   message("+JOYSTICK_SUPPORTED")
   add_definitions(-DJOYSTICK_SUPPORTED)
endif()


if(USE_ASPELL)

message("SEARCH ASPELL")
find_path(ASPELL_INCLUDES aspell.h
         "/usr/include/"
         "/usr/local/"
        )


if(ASPELL_INCLUDES)
  message("+ aspell support")
  add_definitions(-DASPELL_ENABLE)

if(OS2)
  target_link_libraries(tea aspell_dll.a)
else()
  target_link_libraries(tea libaspell.so)
endif()

endif()

endif()


if(USE_PDF)
find_package(PkgConfig)
pkg_check_modules(popplercpp QUIET poppler-cpp)
if(popplercpp_FOUND)
  add_definitions(-DPOPPLER_ENABLE)
  target_link_libraries(tea ${popplercpp_LIBRARIES})
  include_directories(${popplercpp_INCLUDE_DIRS})
  message("+ popplercpp support")
endif()
endif()


if(USE_DJVU)
find_package(PkgConfig)
pkg_check_modules(ddjvuapi QUIET ddjvuapi)
if(ddjvuapi_FOUND)
  add_definitions(-DDJVU_ENABLE)
  target_link_libraries(tea ${ddjvuapi_LIBRARIES})
  include_directories(${ddjvuapi_INCLUDE_DIRS})
  message("+ djvuapi support")
endif()
endif()


#if(UNIX OR MINGW OR OS2)
#    find_package(ZLIB REQUIRED)
#else(UNIX OR MINGW)
#    set(ZLIB_INCLUDE_DIRS "${QT_ROOT}/src/3rdparty/zlib" CACHE STRING "Path to ZLIB headers of Qt")
#    set(ZLIB_LIBRARIES "")
#    if(NOT EXISTS "${ZLIB_INCLUDE_DIRS}/zlib.h")
#        message("Please specify a valid zlib include dir")
#    endif(NOT EXISTS "${ZLIB_INCLUDE_DIRS}/zlib.h")
#endif(UNIX OR MINGW OR OS2)


#if (ZLIB_FOUND)
#    include_directories(${ZLIB_INCLUDE_DIRS} )
#    target_link_libraries(tea ${ZLIB_LIBRARIES} )
#endif( ZLIB_FOUND )

if(hunspell_FOUND)
  include_directories(${hunspell_INCLUDE_DIRS} )
  target_link_libraries(tea ${hunspell_LIBRARIES} )
endif()




#if (Qt6_FOUND)
#target_link_libraries(tea Qt::Core5Compat)
#endif()

target_link_libraries(tea Qt::Widgets Qt::Core)

install (TARGETS tea DESTINATION bin)
install (FILES ${tea_ICONSVG} DESTINATION share/icons/hicolor/scalable/apps)
install (FILES ${tea_ICONPNG32} DESTINATION share/icons/hicolor/32x32/apps)
install (FILES ${tea_ICONPNG48} DESTINATION share/icons/hicolor/48x48/apps)
install (FILES ${tea_ICONPNG64} DESTINATION share/icons/hicolor/64x64/apps)
install (FILES ${tea_ICONPNG128} DESTINATION share/icons/hicolor/128x128/apps)

install (FILES ${tea_DESKTOP} DESTINATION share/applications)