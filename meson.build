project('tea', ['cpp','c'],
         default_options : ['cpp_std=c++11'],
         version : '62.4.0',
         license : 'GPLv3')

add_global_arguments('-DVERSION_NUMBER="62.4.0"', language : 'cpp')
add_global_arguments('-DNOCRYPT="1"', language : 'cpp')
add_global_arguments('-DNOUNCRYPT="1"', language : 'cpp')
add_global_arguments('-DQUAZIP_STATIC="1"', language : 'cpp')

add_global_arguments('-DNOCRYPT="1"', language : 'c')
add_global_arguments('-DNOUNCRYPT="1"', language : 'c')

#harcoded for now
moc_params = ['-DQT_VERSION=0x050000']


if build_machine.system() == 'linux'
moc_params += ['-DQ_OS_LINUX=1', '-DQ_OS_UNIX=1']
endif

compiler = meson.get_compiler('cpp')

if compiler.has_header('/usr/include/linux/joystick.h')
  moc_params += ['-DJOYSTICK_SUPPORTED=1']
  add_global_arguments('-DJOYSTICK_SUPPORTED=1', language : 'cpp')
endif


aspell_dep = compiler.find_library('aspell', required : false)
if get_option('aspell').enabled()
if aspell_dep.found()
   add_global_arguments('-DASPELL_ENABLE=1', language : 'cpp')
   moc_params += ['-DASPELL_ENABLE=1']
endif
endif

hunspell_dep = dependency('hunspell', required : false)
if hunspell_dep.found()
   add_global_arguments('-DHUNSPELL_ENABLE=1', language : 'cpp')
   moc_params += ['-DHUNSPELL_ENABLE=1']
endif


poppler_dep = dependency('poppler-qt5', required : false)
if get_option('pdf').enabled()
if poppler_dep.found()
   add_global_arguments('-DPOPPLER_ENABLE=1', language : 'cpp')
   moc_params += ['-DPOPPLER_ENABLE=1']
endif
endif

ddjvuapi_dep = dependency('ddjvuapi', required : false)
if get_option('djvu').enabled()
if ddjvuapi_dep.found()
   add_global_arguments('-DDJVU_ENABLE=1', language : 'cpp')
   moc_params += ['-DDJVU_ENABLE=1']
endif
endif


zlib_dep = dependency('zlib', required : true)

qt5_dep = dependency('qt5', modules : ['Core', 'Gui', 'Widgets']) 

qt5 = import('qt5') 

tea_headers_moc = [
        'quagzipfile.h',
        'quaziodevice.h',
        'quazipfile.h',
        'tea.h',
        'todo.h',
        'document.h',
        'calendar.h',
    'fman.h',
    'shortcuts.h',
    'logmemo.h',
    'img_viewer.h',
    'tio.h',
    'tzipper.h',
    'single_application_shared.h',
    'myjoystick.h',
    'pugixml.hpp',
    'pugiconfig.hpp']


src_processed = qt5.preprocess(
   moc_headers : tea_headers_moc,
   moc_extra_arguments: moc_params,
   qresources  : 'resources.qrc'
)


tea_source = ['tea.cpp',
              'main.cpp',
              'todo.cpp',
              'textproc.cpp',
              'libretta_calc.cpp',
              'wavinfo.cpp',
              'calendar.cpp',
              'gui_utils.cpp',
              'document.cpp',
              'utils.cpp',
    'spellchecker.cpp',
    'fman.cpp',
    'shortcuts.cpp',
    'logmemo.cpp',
    'img_viewer.cpp',
    'tio.cpp',
    'tzipper.cpp',
    'single_application_shared.cpp',
    'exif_reader.cpp',
    'myjoystick.cpp',
    'qioapi.cpp',
    'quagzipfile.cpp',
    'quaziodevice.cpp',
    'quazip.cpp',
    'quazipdir.cpp',
    'quazipfile.cpp',
    'quazipfileinfo.cpp',
    'quazipnewinfo.cpp',
    'unzip.c',
    'zip.c',
    'pugixml.cpp']


tea_exe = executable ('tea',
  sources : [src_processed, tea_source], 
  install : true,
  dependencies : [qt5_dep, hunspell_dep, aspell_dep, zlib_dep, poppler_dep, ddjvuapi_dep]
  )


install_data(['icons/32/tea.png'],
             install_dir : 'share/icons/hicolor/32x32/apps')

install_data(['icons/48/tea.png'],
             install_dir : 'share/icons/hicolor/48x48/apps')

install_data(['icons/64/tea.png'],
             install_dir : 'share/icons/hicolor/64x64/apps')

install_data(['icons/128/tea.png'],
             install_dir : 'share/icons/hicolor/128x128/apps')

install_data(['icons/svg/tea.svg'],
             install_dir : 'share/icons/hicolor/scalable/apps')

install_data(['desktop/tea.desktop'],
             install_dir : 'share/applications')

