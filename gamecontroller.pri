#QT = core
#load(qt_build_config)
#TARGET = QtGameController
#load(qt_module)

DEFINES += QGAMECONTROLLER_LIBRARY

#QMAKE_DOCS = $$PWD/doc/qtserialport.qdocconf

#win32-g++:LIBS += -ldinput8 -ldxguid

#win32:CONFIG += c++11

#mac:LIBS += -framework IOKit
#mac:LIBS += -framework CoreFoundation

SOURCES +=  qgamecontroller.cpp

unix:SOURCES +=  qgamecontroller_linux.cpp
#macx:SOURCES +=  qgamecontroller_mac.cpp
#win32:SOURCES +=  qgamecontroller_win.cpp

PUBLIC_HEADERS += qgamecontroller.h
PRIVATE_HEADERS +=  qgamecontroller_p.h
HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS
