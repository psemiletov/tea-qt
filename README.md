# TEA #

TEA is a C++, Qt(4,5,6) text editor with the hundreds of features for Linux, *BSD, Mac, Windows, OS/2 and Haiku. 

Home site > 
http://tea.ourproject.org

Development >
https://github.com/psemiletov/tea-qt

My hot AUR package > 
https://aur.archlinux.org/packages/tea-qt-git/

Donate > 

PayPal: peter.semiletov@gmail.com

BuyMeACoffee: https://www.buymeacoffee.com/semiletov

Patreon: https://www.patreon.com/semiletov


Communities >

https://t.me/teaqt

https://www.facebook.com/groups/766324686841748/


## CONTENTS OF THIS README: ##

01 - INSTALLATION FROM THE SOURCE

02 - NOTES FOR PACKAGE MAINTAINERS

03 - NOTE FOR UBUNTU USERS

04 - LICENSE NOTES


### 01: INSTALLATION FROM THE SOURCE ###

You can install TEA from the source in many ways, using build systems qmake/make, cmake/make, cmake/ninja. But first, you need to install some development libraries.

**Mandatory:**

Qt 4.8 or Qt 5.4+ or Qt 6

**Optional:**

TEA has some optional dependencies, those extends the functionality and can be turned on/off during pre-build configuration. Here is a list with library names and cmake command line switches:


libaspell: -DUSE_ASPELL=ON //enables Aspell spell checker, OFF by default  

nuspell: -DUSE_NUSPELL=ON //enables Nuspell engine, that uses Hunspell dictionaries, OFF by default  

libhunspell: -DUSE_HUNSPELL=ON //enables Hunspell engine, ON by default  

poppler-cpp: -DUSE_PDF=ON //to read the text from PDF, OFF by default  

ddjvuapi: -DUSE_DJVU=ON //to read the text from DJVU,  OFF by default  

speech-dispatcher: -DUSE_SPEECH=ON //makes TEA the speech-dispatcher client, to speach the text. OFF by default, and can be turned of when build with it. 

**Note for FreeBSD users**: you need the pkgconf package - pkg install pkgconf

Which build system you should use?

Use qmake for: Qt 4, old distros and Windows. Use cmake for modern distros. cmake is the mainline build system for TEA. 


#### 01.01 CMAKE #### 

With cmake, TEA supports Qt5 and Qt6 build.

If you want to build and install TEA with cmake + make, run at the TEA source dir:

mkdir b  
cd b  
cmake ..  
make  
make install (as root or with sudo)  


To build and install TEA with cmake/ninja and GCC, do:

mkdir b  
cd b  
cmake -GNinja ..  
ninja  
ninja install  

By default, cmake builds TEA without some features: printer and aspell support, libpoppler and djvuapi. To enable them, use from the build directory:

cmake .. -DUSE_ASPELL=ON -DUSE_PRINTER=ON -DUSE_PDF=ON -DUSE_DJVU=ON  

If the Qt5 and Qt6 both are present on the system, use CMAKE_PREFIX_PATH variable to set the path to the QtN. Otherwise, Qt6 will be prefered.

   Examples:

cmake -DCMAKE_PREFIX_PATH=/usr/lib/qt ..  
//usr/lib/qt is the directory with qt5  

cmake -DCMAKE_PREFIX_PATH=/usr/lib/qt6 ..  
//usr/lib/qt6 is the directory with qt6  

cmake -DCMAKE_PREFIX_PATH=$HOME/Qt/6.0.0/gcc_64/lib/cmake ..  
//here we point to the locally installed Qt6  


#### 01.02 QMAKE ####

QMake building system is outdated and supported by TEA just as legacy and to make TEA works at old distros or systems (Windows XP for example).

TEA's qmake project file supports Qt4 and Qt5 build.

With qmake to build is simple:

qmake  
make  
make install (as root or with sudo)  

To make some source configuration (with qmake), use CONFIG variable at qmake command line parameter. For example:

qmake "CONFIG+=useclang" "CONFIG+=noaspell"  

You can use some values:

nosingleapp - do not build TEA with the single application mode support  
nodesktop - do not install desktop files and icons  
useclang - TEA will be compiled with Clang  
noaspell - disable the Aspell (if you have it installed, but  do not want to compile TEA with Aspell support)  
nohunspell - disable Hunspell for TEA  
usepoppler - use libpoppler-cpp for PDF text layer import. DISABLED by default  
usedjvu - use libdjvulibre to read DJVU files text (read only). DISABLED by default  
noprinter - disable printing support  

** Notes: **

1. If you have installed both Qt4 and Qt5, use the qmake from Qt4 or Qt5 to configure TEA with exact version of QT. The common solution is to make symlink to qmake from Qt5 and name it qmake5, then use qmake5 instead of the usual qmake.

2. If the context menus in TEA are not localized, install the qttranslations or qt-translations package from your distro's repository.


/* Some outdated snippets for Ubuntu.

Basic snippet for Ubuntu users (Qt5 build) - run this from Terminal at the TEA source directory (unpacked):

sudo apt-get install g++ pkg-config  
sudo apt-get install zlib1g-dev libaspell-dev libhunspell-dev  
sudo apt-get install qt5-default qttools5-dev-tools  
qmake  
make  
sudo make install  


Snippet for very old Ubuntu: (Qt4 build):

sudo apt-get install g++ pkg-config  
sudo apt-get install zlib1g-dev libaspell-dev libhunspell-dev  
sudo apt-get install libqt4-dev qt4-dev-tools  
qmake  
make  
sudo make install  
*/


### 02: NOTES FOR PACKAGE MAINTAINERS ### 

0. Thank you for packaging TEA!

1. Altough TEA has two home sites, it is better to use Github releases as the source:
https://github.com/psemiletov/tea-qt/archive/$pkgver.tar.gz
   
Please note, that TEA source dir after unpacking will be tea-qt-${pkgver}

2. TEA after the compilation is a single binary file (with embedded resources).

3. TEA supports 3 build systems:

  **qmake** - the traditional one, good for Qt4-Win32-OS/2-Slackware builds. TEA's qmake project file is old and obscure. 

  **cmake** - good for Qt5/Qt6 build, the reference number one for TEA. I recommend to use cmake to build TEA package.


4. For the qmake build, to override the default installation path (/usr/local, with binary at /usr/local/bin) use:

qmake PREFIX=your_path  
make  
make install  


### 03: NOTE FOR UBUNTU USERS ###

User defined hotkeys may not work due to Qt5 and Unity global menu feature. To remove global menu support in Qt5 apps, do

sudo apt-get autoremove appmenu-qt5

or, if you want to remove also GTK global menus, use:

sudo apt-get autoremove appmenu-gtk appmenu-gtk3 appmenu-qt5


### 04: LICENSE NOTES ###

TEA code is licensed under GPL V3 and, partially, as a Public Domain. TEA media (images, etc), manuals and translations are public domain. Note to contributors - please put your translations into the public domain or GPL.
