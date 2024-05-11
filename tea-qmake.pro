VERSION = 63.0.1

os2: {
DEFINES += 'VERSION_NUMBER=\'"63.0.1"\''
} else: {
  DEFINES += 'VERSION_NUMBER=\\\"$${VERSION}\\\"'
}


exists("/usr/include/linux/joystick.h") {
      message ("+JOYSTICK_SUPPORTED")
      DEFINES += JOYSTICK_SUPPORTED
}


#QMAKE_CXXFLAGS += -std=c++11

#DEFINES += NOCRYPT \
#           NOUNCRYPT \
#           QUAZIP_STATIC


USE_ASPELL = true
USE_HUNSPELL = true
USE_PRINTER = true

win32:{
       isEmpty(PREFIX) {
       PREFIX = /usr/local/bin
       }

TARGET = bin/tea
target.path = $$PREFIX
}


os2:{

isEmpty(PREFIX) {
PREFIX = /usr/local/bin
}

TARGET = bin/tea
target.path = $$PREFIX
}


unix: {
       isEmpty(PREFIX) {
                        PREFIX = /usr/local
                       }

       PREFIX = $$replace(PREFIX, bin,)
       TARGET = bin/tea
       target.path = $$PREFIX/bin
 
       desktop.path = $$PREFIX/share/applications
       desktop.files = desktop/tea.desktop

       icon128.path = $$PREFIX/share/icons/hicolor/128x128/apps/
       icon128.files += icons/128/tea.png

       icon64.path = $$PREFIX/share/icons/hicolor/64x64/apps/
       icon64.files += icons/64/tea.png

       icon48.path = $$PREFIX/share/icons/hicolor/48x48/apps/
       icon48.files += icons/48/tea.png

       icon32.path = $$PREFIX/share/icons/hicolor/32x32/apps/
       icon32.files += icons/32/tea.png

       iconsvg.path = $$PREFIX/share/icons/hicolor/scalable/apps/
       iconsvg.files += icons/svg/tea.svg
      }


nohunspell {
            USE_HUNSPELL = false
           }

noaspell {
          USE_ASPELL = false
         }

DEFINES += PRINTER_ENABLE

noprinter{
DEFINES -= PRINTER_ENABLE
message ("Printer OFF")
}


useclang{
    message ("Clang enabled")
    QMAKE_CC=clang
    QMAKE_CXX=clang++
    QMAKE_CXXFLAGS += -std=c++11
}

SOURCES += src/tea.cpp \
    src/main.cpp \
    src/todo.cpp \
    src/textproc.cpp \
    src/libretta_calc.cpp \
    src/wavinfo.cpp \
    src/calendar.cpp \
    src/gui_utils.cpp \
    src/document.cpp \
    src/utils.cpp \
    src/spellchecker.cpp \
    src/fman.cpp \
    src/shortcuts.cpp \
    src/logmemo.cpp \
    src/img_viewer.cpp \
    src/tio.cpp \
    src/single_application_shared.cpp \
    src/exif_reader.cpp \
    src/myjoystick.cpp \
    src/pugixml.cpp \
    src/zip.c \
    src/enc.cpp


HEADERS += src/tea.h \
    src/todo.h \
    src/document.h \
    src/utils.h \
    src/textproc.h \
    src/calendar.h \
    src/libretta_calc.h \
    src/spellchecker.h \
    src/fman.h \
    src/shortcuts.h \
    src/logmemo.h \
    src/img_viewer.h \
    src/gui_utils.h \
    src/wavinfo.h \
    src/tio.h \
    src/single_application_shared.h \
    src/exif_reader.h \
    src/myjoystick.h \
    src/pugixml.hpp \
    src/pugiconfig.hpp \
    src/miniz.h \
    src/zip.h \
    src/enc.h



TEMPLATE = app

CONFIG += warn_on \
    thread \
    qt \
    release \
    link_pkgconfig


QT += core
QT += gui

greaterThan(QT_MAJOR_VERSION, 4) {
       QT += widgets
       QT += printsupport

noprinter{
       QT -= printsupport
}

   } else {
#QT += blah blah blah
   }



nosingleapp{
       DEFINES += NO_SINGLE_APP
       message ("No single application mode")
}


INSTALLS += target desktop icon128 icon64 icon48 icon32 iconsvg

nodesktop{
INSTALLS -= desktop icon128 icon64 icon48 icon32 iconsvg
}

RESOURCES += resources.qrc
TRANSLATIONS = translations/ru.ts \
               translations/de.ts \
               translations/fr.ts \
               translations/pl.ts


DISTFILES += ChangeLog \
    COPYING \
    README \
    NEWS \
    NEWS-RU \
    AUTHORS \
    TODO \
    hls/* \
    palettes/* \
    encsign/* \
    images/* \
    desktop/* \
    text-data/* \
    translations/* \
    manuals/en.html \
    manuals/ru.html \
    manuals/pl.html \
    themes/Cotton/stylesheet.css \
    themes/Plum/stylesheet.css \
    themes/Smaragd/stylesheet.css \
    themes/TEA/stylesheet.css \
    themes/Turbo/stylesheet.css \
    themes/Turbo/icons/* \
    themes/Vegan/stylesheet.css \
    themes/Yagodnaya/stylesheet.css \
    icons/32/tea.png \
    icons/48/tea.png \
    icons/64/tea.png \
    icons/128/tea.png \
    icons/svg/tea.svg \
    icons/current-list.png \
    icons/edit-copy-active.png \
    icons/edit-copy.png \
    icons/edit-cut-active.png \
    icons/edit-cut.png \
    icons/edit-paste-active.png \
    icons/edit-paste.png \
    icons/file-new.png \
    icons/file-open-active.png \
    icons/file-open.png \
    icons/file-save-active.png \
    icons/file-save-as.png \
    icons/file-save.png \
    icons/labels.png \
    icons/tea_icon_v2.png \
    icons/tea-icon-v3-01.png \
    icons/tea-icon-v3-02.png \
    icons/tea-icon-v3-03.png


unix: {
#       system (pkg-config --exists zlib) {
#               message ("Zlib found")
#               PKGCONFIG += zlib
#               }

       contains (USE_ASPELL,true) {
                exists ("/usr/include/aspell.h") {
                        message ("ASpell enabled")
                        LIBS += -laspell
                        DEFINES += ASPELL_ENABLE
                       }
                }


       contains (USE_HUNSPELL,true) {
                 system (pkg-config --exists hunspell) {
                         message ("hunspell enabled")
                         PKGCONFIG += hunspell
                         DEFINES += HUNSPELL_ENABLE
                         }
                }


usepoppler{
system(pkg-config --exists poppler-cpp) {
    message ("Poppler enabled")
        PKGCONFIG += poppler-cpp
        DEFINES += POPPLER_ENABLE
        }
}


usedjvu{
system(pkg-config --exists ddjvuapi) {
            message ("djvu enabled")
            PKGCONFIG += ddjvuapi
            DEFINES += DJVU_ENABLE
        }
}

}


win32: {

greaterThan(QT_MAJOR_VERSION, 4) {


       message ("QT > 4")

#       LIBS += zlib1.dll

        contains(USE_ASPELL,true)
                 {
                  exists ("C:\\Qt\\Qt5.3.1\\Tools\\mingw482_32\\include\\aspell.h")
                         {
                          message ("ASpell enabled")
                          LIBS += -laspell-15
                          DEFINES += ASPELL_ENABLE
                          }
                  }


        contains(USE_HUNSPELL,true)
                 {
                  exists ("c:\\Qt\\Qt5.3.1\\5.3\\mingw482_32\\include\\hunspell\\hunspell.hxx")

                         {
                          message ("hunspell enabled")
#                          LIBS += c:\dev\hunspell-mingw-master\lib\hunspell.dll
                      #     LIBS += libhunspell.dll
                          LIBS += hunspell.dll
                          DEFINES += HUNSPELL_ENABLE
                         }
                }

  } else {
#   LIBS += zlib1.dll

#   INCLUDEPATH+=c:\\Qt\\4.8.4\\src\\3rdparty\\zlib

        contains(USE_ASPELL,true)
                 {
                  exists ("C:\\MinGw\\include\\aspell.h")
                         {
                          message ("ASpell enabled")
                          LIBS += -laspell-15
                          DEFINES += ASPELL_ENABLE
                         }
                  }


        contains(USE_HUNSPELL,true)
                 {
                  exists ("C:\\MinGw\\include\\hunspell\\hunspell.hxx")
                         {
                          message ("hunspell enabled")
                          LIBS += libhunspell.dll
                          DEFINES += HUNSPELL_ENABLE
                         }
                }
}
}


os2: {

#system(pkg-config --exists zlib) {
#    message ("Zlib found")
#        PKGCONFIG += zlib
#        }


contains(USE_ASPELL,true){
exists("c:/usr/include/aspell.h") {
      message ("ASpell enabled")
      LIBS += -laspell_dll
      DEFINES += ASPELL_ENABLE
}
}



contains(USE_HUNSPELL,true){
system(pkg-config --exists hunspell) {
message ("hunspell enabled")
        PKGCONFIG += hunspell
            DEFINES += HUNSPELL_ENABLE
                }
                }

usepoppler{
system(pkg-config --exists poppler-cpp) {
    message ("Poppler enabled")
        PKGCONFIG += poppler-cpp
        DEFINES += POPPLER_ENABLE
        }
}


usedjvu{
system(pkg-config --exists ddjvuapi) {
    message ("djvu enabled")
        PKGCONFIG += ddjvuapi
            DEFINES += DJVU_ENABLE
        }
}

}
