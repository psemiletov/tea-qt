VERSION = 45.0.2

os2: {
DEFINES += 'VERSION_NUMBER=\'"45.0.2"\''
} else: {
  DEFINES += 'VERSION_NUMBER=\\\"$${VERSION}\\\"'
}


DEFINES += NOCRYPT \
          NOUNCRYPT

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


unix:{

isEmpty(PREFIX) {
PREFIX = /usr/local
}

#old PREFIX compatibility hack
#message($$replace(PREFIX, bin,))
#message ($$PREFIX)
PREFIX = $$replace(PREFIX, bin,)
#message ($$PREFIX)
#


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


nohunspell{
USE_HUNSPELL = false
}

noaspell{
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



SOURCES += rvln.cpp \
    main.cpp \
    todo.cpp \
    textproc.cpp \
    libretta_calc.cpp \
    wavinfo.cpp \
    calendar.cpp \
    gui_utils.cpp \
    document.cpp \
    utils.cpp \
    spellchecker.cpp \
    fman.cpp \
    shortcuts.cpp \
    logmemo.cpp \
    img_viewer.cpp \
    fontbox.cpp \
    tio.cpp \
    tzipper.cpp \
    ioapi.c \
    quazip.cpp \
    quazipfile.cpp \
    quazipnewinfo.cpp \
    unzip.c \
    zip.c \
    single_application.cpp \
    single_application_shared.cpp \
    exif.cpp \
    myjoystick.cpp


HEADERS += rvln.h \
    todo.h \
    document.h \
    utils.h \
    textproc.h \
    calendar.h \
    libretta_calc.h \
    spellchecker.h \
    fman.h \
    shortcuts.h \
    logmemo.h \
    img_viewer.h \
    gui_utils.h \
    wavinfo.h \
    zconf.h \
    zlib.h \
    tio.h \
    fontbox.h \
    tzipper.h \
    ioapi.h \
    quazip.h \
    quazipfile.h \
    quazipfileinfo.h \
    quazipnewinfo.h \
    unzip.h \
    zip.h \
    single_application.h \
    single_application_shared.h \
    exif.h \
    myjoystick.h
  


TEMPLATE = app

CONFIG += warn_on \
    thread \
    qt \
    release \
    link_pkgconfig

QT += core
QT += gui
QT += network
#QT += svg

CONFIG-=qml_debug

greaterThan(QT_MAJOR_VERSION, 4) {
       QT += widgets
       QT += printsupport
       QT += qml
       QT += quick

noprinter{
       QT -= printsupport
}

    DEFINES += USE_QML_STUFF
   } else {
#QT += blah blah blah
   }



noqml{
       QT -= qml
       QT -= quick
       DEFINES -= USE_QML_STUFF
       message ("QML stuff is off")
}


nosingleapp{
       DEFINES += NO_SINGLE_APP
       message ("No single application mode")
}

   
INSTALLS += target desktop icon128 icon64 icon48 icon32 iconsvg

nodesktop{
INSTALLS -= desktop icon128 icon64 icon48 icon32 iconsvg
}

RESOURCES += rlvn.qrc
TRANSLATIONS = translations/tea_ru.ts \
               translations/tea_de.ts \
               translations/tea_fr.ts


DISTFILES += ChangeLog \
    COPYING \
    README \
    NEWS \
    NEWS-RU \
    AUTHORS \
    TODO \
    INSTALL \
    hls/* \
    palettes/* \
    encsign/* \
    images/* \
    desktop/* \
    text-data/* \
    translations/* \
    manuals/en.html \
    manuals/ru.html \
    manuals/plugins/readme-en.txt \
    manuals/plugins/examples/click-and-close/main.qml \
    manuals/plugins/examples/run-and-close/main.qml \
    manuals/plugins/examples/settings-via-qmap/main.qml \
    manuals/plugins/examples/use-fif/main.qml \
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
    icons/create-dir.png \
    icons/create-dir.png \
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
icons/fn-spell-check.png \
icons/go.png \
icons/home.png \
icons/labels.png \
icons/refresh.png \
icons/search_find.png \
icons/tea_icon_v2.png \
icons/tea-icon-v3-01.png \
icons/tea-icon-v3-02.png \
icons/tea-icon-v3-03.png




unix:  {
        LIBS += -lz


#omp{
#    QMAKE_CXXFLAGS += -fopenmp
#    QMAKE_LFLAGS +=  -fopenmp
#    LIBS += -lgomp -lpthread
#    message ("OMP enabled")
#}


contains(USE_ASPELL,true){
exists("/usr/include/aspell.h") {
    message ("ASpell enabled")
    LIBS += -laspell
    DEFINES += ASPELL_ENABLE
    DEFINES += SPELLCHECK_ENABLE
}
}



contains(USE_HUNSPELL,true){
system(pkg-config --exists hunspell) {
message ("hunspell enabled")
        PKGCONFIG += hunspell
            DEFINES += HUNSPELL_ENABLE
                DEFINES += SPELLCHECK_ENABLE
                }
                }

                
usepoppler{
system(pkg-config --exists poppler-qt5) {
    message ("Poppler enabled")
        PKGCONFIG += poppler-qt5
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

       LIBS += zlib1.dll

        contains(USE_ASPELL,true)
                 {
                  exists ("C:\\Qt\\Qt5.3.1\\Tools\\mingw482_32\\include\\aspell.h")
                         {
                          message ("ASpell enabled")
                          LIBS += -laspell-15
                          DEFINES += ASPELL_ENABLE
                          DEFINES += SPELLCHECK_ENABLE
                          }
                  }


        contains(USE_HUNSPELL,true)
                 {
                  exists ("C:\\Qt\\Qt5.3.1\\Tools\\mingw482_32\\include\\hunspell\\hunspell.hxx")
                         {

                          message ("hunspell enabled")


                          LIBS += c:\dev\hunspell-mingw-master\lib\hunspell.dll
                      #     LIBS += libhunspell.dll


                          DEFINES += HUNSPELL_ENABLE
                          DEFINES += SPELLCHECK_ENABLE
                         }
                }

  } else {
   LIBS += zlib1.dll

        contains(USE_ASPELL,true)
                 {
                  exists ("C:\\MinGw\\include\\aspell.h")
                         {
                          message ("ASpell enabled")
                          LIBS += -laspell-15
                          DEFINES += ASPELL_ENABLE
                          DEFINES += SPELLCHECK_ENABLE
                          }
                  }


        contains(USE_HUNSPELL,true)
                 {
                  exists ("C:\\MinGw\\include\\hunspell\\hunspell.hxx")
                         {
                          message ("hunspell enabled")
                          LIBS += libhunspell.dll
                          DEFINES += HUNSPELL_ENABLE
                          DEFINES += SPELLCHECK_ENABLE
                         }
                }
}
}



os2: {


usepoppler{
exists("C:/usr/include/poppler/qt4/poppler-qt4.h") {
     message ("DJVU enabled")
         PKGCONFIG += poppler-qt4
             DEFINES += POPPLER_ENABLE
}
}


usedjvu{
exists("C:/usr/include/libdjvu/ddjvuapi.h") {
exists("C:/usr/include/libdjvu/miniexp.h") {
     message ("djvu enabled")
         PKGCONFIG += ddjvuapi
             DEFINES += DJVU_ENABLE
}
}
}

contains(USE_ASPELL,true){
exists("c:/usr/include/aspell.h") {
      message ("ASpell enabled")
      LIBS += -laspell_dll
      DEFINES += ASPELL_ENABLE
      DEFINES += SPELLCHECK_ENABLE
contains(USE_HUNSPELL,true)
}
}


exists("c:/extras/include/hunspell/hunspell.hxx") {
      message ("hunspell enabled")
      LIBS += -lhunspell-1.4
      PKGCONFIG += hunspell
      DEFINES += HUNSPELL_ENABLE
      DEFINES += SPELLCHECK_ENABLE
}
}
