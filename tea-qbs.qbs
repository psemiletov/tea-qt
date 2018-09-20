/*
Cannot test because qbs is buggy on my system

*/

import qbs 1.0

Application  {


   Depends {
        name : "cpp","Qt.core","Qt.gui", "Qt.widgets", "Qt.network", "Qt.printsupport", "Qt.qml", "Qt.quick"
    }

    name  : "tea"
    files : [
             "rvln.cpp", 
             "main.cpp",
             "todo.cpp",
             "textproc.cpp",
"libretta_calc.cpp",
"wavinfo.cpp",
"calendar.cpp",
"gui_utils.cpp",
"document.cpp",
"utils.cpp",
"spellchecker.cpp",
"fman.cpp",
"shortcuts.cpp",
"logmemo.cpp",
"img_viewer.cpp",
"fontbox.cpp",
"tio.cpp",
"tzipper.cpp",
"single_application.cpp",
"single_application_shared.cpp",
"exif.cpp",
"myjoystick.cpp",

"qioapi.cpp",
"quaadler32.cpp",
"quacrc32.cpp",
"quagzipfile.cpp",
"quaziodevice.cpp",
"quazip.cpp",
"quazipdir.cpp",
"quazipfile.cpp",
"quazipfileinfo.cpp",
"quazipnewinfo.cpp",
"unzip.c",
"zip.c",
"rvln.h",
"todo.h",
"document.h",
"utils.h",
"textproc.h",
"calendar.h",
"libretta_calc.h",
"spellchecker.h",
"fman.h",
"shortcuts.h",
"logmemo.h",
"img_viewer.h",
"gui_utils.h",
"wavinfo.h",
"tio.h",
"fontbox.h",
"tzipper.h",
"single_application.h",
"single_application_shared.h",
"exif.h",
"myjoystick.h",
"minizip_crypt.h",
"ioapi.h",
"quaadler32.h",
"quachecksum32.h",
"quacrc32.h",
"quagzipfile.h",
"quaziodevice.h",
"quazipdir.h",
"quazipfile.h",
"quazipfileinfo.h",
"quazip_global.h",
"quazip.h",
"quazipnewinfo.h",
"unzip.h",
"zip.h"
]


    Group {
        name: "app"
        fileTagsFilter: "application"
        qbs.install: true
        qbs.installDir: "bin"
    }
}