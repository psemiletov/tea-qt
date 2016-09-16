VERSION = 43.0.0

os2: {
DEFINES += 'VERSION_NUMBER=\'"42.0.0"\''
} else: {
  DEFINES += 'VERSION_NUMBER=\\\"$${VERSION}\\\"'
}


DEFINES += NOCRYPT \
          NOUNCRYPT

USE_POPPLER = false
USE_ASPELL = true
USE_HUNSPELL = true
USE_PRINTER = true


isEmpty(PREFIX) {
PREFIX = /usr/local
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
    QMAKE_CXX=clang
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
    exif.cpp

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
    exif.h



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

greaterThan(QT_MAJOR_VERSION, 4) {
       QT += widgets
       QT += printsupport
       QT += qml
       QT += quick

   } else {
#QT += blah blah blah
   }

TARGET = tea
target.path = $$PREFIX/bin

INSTALLS += target
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
    icons/* \
    palettes/* \
    encsign/* \
    images/* \
    manuals/en.html \
    manuals/ru.html \
    manuals/plugins/readme-en.txt \
    manuals/plugins/examples/click-and-close/main.qml \
    manuals/plugins/examples/run-and-close/main.qml \
    manuals/plugins/examples/settings-via-qmap/main.qml \
    manuals/plugins/examples/use-fif/main.qml \
    text-data/* \
    translations/* \
    themes/Cotton/stylesheet.css \
    themes/Plum/stylesheet.css \
    themes/Smaragd/stylesheet.css \
    themes/TEA/stylesheet.css \
    themes/Turbo/stylesheet.css \
    themes/Turbo/icons/* \
    themes/Vegan/stylesheet.css \
    themes/Ягодная/stylesheet.css 


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
exists("/usr/include/hunspell/hunspell.hxx") { 
    message ("hunspell enabled")
        PKGCONFIG += hunspell
            DEFINES += HUNSPELL_ENABLE
                DEFINES += SPELLCHECK_ENABLE
                }
                }


#contains(USE_POPPLER,true){

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



os2: {
contains(USE_ASPELL,true){
exists("q:/usr/include/aspell.h") {
     message ("ASpell enabled")
     LIBS += -laspell
     DEFINES += ASPELL_ENABLE
     DEFINES += SPELLCHECK_ENABLE
contains(USE_HUNSPELL,true)
}
}

{
exists("q:/usr/include/hunspell/hunspell.hxx") {
     message ("hunspell enabled")
     LIBS += -lhunspell-1.3
     PKGCONFIG += hunspell
     DEFINES += HUNSPELL_ENABLE
     DEFINES += SPELLCHECK_ENABLE
}
}

}

