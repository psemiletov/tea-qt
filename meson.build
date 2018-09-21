project('tea', ['cpp','c'],
         default_options : ['cpp_std=c++11'],
         version : '46.1.0',
         license : 'GPLv3')

add_global_arguments('-DVERSION_NUMBER="46.1.0"', language : 'cpp')
add_global_arguments('-DUSE_QML_STUFF=1', language : 'cpp')

#harcoded for now
moc_params = ['-DQT_VERSION=0x050000', '-DUSE_QML_STUFF=1']

if build_machine.system() == 'linux'
moc_params += ['-DQ_OS_LINUX=1', '-DQ_OS_UNIX=1']
endif


hunspell_dep = dependency('hunspell', required : false)
if hunspell_dep.found()
   add_global_arguments('-DSPELLCHECK_ENABLE=1', language : 'cpp')
   add_global_arguments('-DHUNSPELL_ENABLE=1', language : 'cpp')
   moc_params += ['-DSPELLCHECK_ENABLE=1']
   moc_params += ['-DHUNSPELL_ENABLE=1']
endif

poppler_dep = dependency('poppler-qt5', required : false)
if poppler_dep.found()
   add_global_arguments('-DPOPPLER_ENABLE=1', language : 'cpp')
   moc_params += ['-DPOPPLER_ENABLE=1']
endif

ddjvuapi_dep = dependency('ddjvuapi', required : false)
if ddjvuapi_dep.found()
   add_global_arguments('-DDJVU_ENABLE=1', language : 'cpp')
   moc_params += ['-DDJVU_ENABLE=1']
endif


zlib_dep = dependency('zlib', required : true)

qt5_dep = dependency('qt5', modules : ['Core', 'Gui', 'Widgets', 'Network', 'Qml', 'Quick']) 
qt5 = import('qt5') 


tea_headers_moc = [
        'quagzipfile.h',
        'quaziodevice.h',
        'quazipfile.h',
        'rvln.h',
    'todo.h',
    'document.h',
    'calendar.h',
    'fman.h',
    'shortcuts.h',
    'logmemo.h',
    'img_viewer.h',
    'tio.h',
    'fontbox.h',
    'tzipper.h',
    'single_application.h',
    'single_application_shared.h',
    'myjoystick.h']


src_processed = qt5.preprocess(
  moc_headers : tea_headers_moc,
   moc_extra_arguments: moc_params,
  qresources  : 'resources.qrc'
)


tea_source = ['rvln.cpp',
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
    'fontbox.cpp',
    'tio.cpp',
    'tzipper.cpp',
    'single_application.cpp',
    'single_application_shared.cpp',
    'exif.cpp',
    'myjoystick.cpp',
    'qioapi.cpp',
    'quaadler32.cpp',
    'quacrc32.cpp',
    'quagzipfile.cpp',
    'quaziodevice.cpp',
    'quazip.cpp',
    'quazipdir.cpp',
    'quazipfile.cpp',
    'quazipfileinfo.cpp',
    'quazipnewinfo.cpp',
    'unzip.c',
    'zip.c']


tea_exe = executable ('tea',
  sources : [src_processed, tea_source], 
  install : true,
  dependencies : [qt5_dep, hunspell_dep, zlib_dep, poppler_dep, ddjvuapi_dep]
#  cpp_args : '-std=c++11'  
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
