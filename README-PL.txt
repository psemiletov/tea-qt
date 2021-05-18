ZAWARTOŚĆ NINIEJSZEGO README:
01 - INSTALACJA OD ŹRÓDŁA

02 - UWAGI DLA KONSERWATORÓW PAKIETÓW

03 - UWAGA DLA UŻYTKOWNIKÓW UBUNTU

04 - UWAGI LICENCYJNE

01: INSTALACJA OD ŹRÓDŁA
Możesz zainstalować TEA ze źródła na 4 sposoby, używając systemów budowania qmake / make, meson / ninja, cmake / make, cmake / ninja. Ale najpierw musisz zainstalować kilka bibliotek programistycznych.

Obowiązkowy:

Qt 4.8 lub Qt 5.4+ lub Qt 6, zlib

Opcjonalny:

libaspell (do silnika sprawdzania pisowni), libhunspell (do silnika sprawdzania pisowni), poppler-qt5 lub poppler-qt6 (do czytania tekstu z PDF), ddjvuap (do czytania tekstu z DJVU)

Uwaga dla użytkowników FreeBSD : potrzebujesz pakietu pkgconf - pkg install pkgconf

Którego systemu kompilacji należy użyć?

Użyj qmake dla: Qt 4, starych dystrybucji i Windows. Użyj mezon lub cmake dla nowoczesnych dystrybucji. cmake jest głównym systemem budowania dla TEA.

01.01 CMAKE
Dzięki cmake TEA obsługuje kompilację Qt5 i Qt6.

Jeśli chcesz zbudować i zainstalować TEA za pomocą cmake + make, uruchom na źródle TEA reż:

mkdir b
cd b
cmake ..
make
make install (jako root lub sudo)

Aby zbudować i zainstalować TEA z cmake / ninja i GCC, wykonaj:

mkdir b
cd b
cmake -GNinja ..
ninja
ninja install

Domyślnie cmake buduje TEA bez niektórych funkcji: obsługi drukarki i aspell, libpoppler i djvuapi. Aby je włączyć, użyj z katalogu kompilacji:

cmake .. -DUSE_ASPELL = ON -DUSE_PRINTER = ON -DUSE_PDF = ON -DUSE_DJVU = ON

Jeśli Qt5 i Qt6 są obecne w systemie, użyj zmiennej CMAKE_PREFIX_PATH, aby ustawić ścieżkę do QtN. W przeciwnym razie preferowany będzie Qt6.

Przykłady:


cmake -DCMAKE_PREFIX_PATH=/usr/lib/qt ..
//usr/lib/qt is the directory with qt5

cmake -DCMAKE_PREFIX_PATH=/usr/lib/qt6 ..
//usr/lib/qt6 is the directory with qt6

cmake -DCMAKE_PREFIX_PATH=$HOME/Qt/6.0.0/gcc_64/lib/cmake ..
//tutaj wskazujemy na lokalnie zainstalowany Qt6


01.02 MESON
Dzięki mezonowi TEA obsługuje kompilację Qt5.

Aby zbudować i zainstalować TEA z meson / ninja i GCC, wykonaj:

mkdir b
meson
cd b
ninja
ninja install

Aby zbudować i zainstalować TEA z meson / ninja i CLANG, wykonaj:

mkdir b
CC = clang CXX = clang ++ meson b
cd b
ninja
ninja install

Aby włączyć obsługę ekstrakcji tekstu PDF i DJVU oraz obsługę Aspell (domyślnie wyłączona, a także obsługa drukowania):

mkdir b
meson b
meson configure -Dpdf = włączone -Ddjvu = włączone -Daspell = włączone b
cd b
ninja
ninja install

01.03 QMAKE
Dzięki qmake TEA obsługuje kompilację Qt4 i Qt5.

Z qmake budowanie jest proste:

qmake
make
make install (jako root lub sudo)

Aby dokonać konfiguracji źródła (za pomocą qmake), użyj zmiennej CONFIG w parametrze wiersza poleceń qmake. Na przykład:

qmake "CONFIG + = useclang" "CONFIG + = noaspell"

Możesz użyć kilku wartości:

nosingleapp - nie buduj TEA z obsługą trybu pojedynczej aplikacji
nodesktop - nie instaluj plików pulpitu i ikon
useclang - TEA zostanie skompilowana z Clang
noaspell - wyłącz Aspell (jeśli masz go zainstalowanego, ale nie chcesz kompilować TEA z Aspell support)
nohunspell - wyłącz Hunspell dla TEA
usepoppler - użyj libpoppler-qt5 lub qt4 do importu warstwy tekstowej PDF. WYŁĄCZONE domyślnie
usedjvu - użyj libdjvulibre do czytania tekstu plików DJVU (tylko do odczytu). WYŁĄCZONE domyślnie
noprinter - wyłącz obsługę drukowania

** Uwagi: **

Jeśli zainstalowałeś zarówno Qt4, jak i Qt5, użyj qmake z Qt4 lub Qt5, aby skonfigurować TEA z dokładną wersją QT. Typowym rozwiązaniem jest utworzenie dowiązania symbolicznego do qmake z Qt5 i nazwanie go qmake5, a następnie użycie qmake5 zamiast zwykłego qmake.

Jeśli menu kontekstowe w TEA nie są zlokalizowane, zainstaluj pakiet qttranslations lub qt-Translations z repozytorium swojej dystrybucji.

/ * Podstawowy fragment kodu dla użytkowników Ubuntu (kompilacja Qt5) - uruchom go z Terminala w katalogu źródłowym TEA (rozpakowany):

sudo apt-get install g ++ pkg-config
sudo apt-get install zlib1g-dev libaspell-dev libhunspell-dev
sudo apt-get install qt5-default qttools5-dev-tools
sudo apt-get install libqt5qml5 libqt5quick5 qtdeclarative5-dev
qmake
make
sudo make zainstalować

Snippet dla użytkowników Ubuntu (kompilacja Qt4):

sudo apt-get install g ++ pkg-config
sudo apt-get install zlib1g-dev libaspell-dev libhunspell-dev
sudo apt-get install libqt4-dev qt4-dev-tools
qmake
make
sudo make install
* /

02: UWAGI DLA KONSERWATORÓW PAKIETÓW
Dziękujemy za opakowanie TEA!

Chociaż TEA ma dwie strony domowe, lepiej jest użyć wersji Github jako źródła: https://github.com/psemiletov/tea-qt/archive/$pkgver.tar.gz

Pamiętaj, że katalog źródłowy TEA po rozpakowaniu będzie wyglądał następująco: tea-qt - $ {pkgver}

TEA po kompilacji to pojedynczy plik binarny (z osadzonymi zasobami).

TEA obsługuje 3 systemy kompilacji:

qmake - tradycyjny, dobry dla kompilacji Qt4-Win32-OS / 2-Slackware. Plik projektu qmake firmy TEA jest stary i niejasny.

cmake - dobre dla kompilacji Qt5 / Qt6, referencyjne dla TEA. Polecam użyć cmake do zbudowania pakietu TEA.

mezon - używam go wewnętrznie. Nie ma obsługi drukarki.

W przypadku kompilacji qmake, aby zastąpić domyślną ścieżkę instalacyjną (/ usr / local, binarną w / usr / local / bin) podłoże:
qmake PREFIX = twoja_ścieżka
make
make install

03: UWAGA DLA UŻYTKOWNIKÓW UBUNTU
Skróty klawiszowe zdefiniowane przez użytkownika mogą nie działać z powodu funkcji globalnego Qt5 i Jedności. Aby skasować menu globalnego w aplikacjach Qt5, zrób

sudo apt-get autoremove appmenu-qt5

lub, jeśli chcesz usunąć globalne menu GTK, gra:

sudo apt-get autorove appmenu-gtk appmenu-gtk3 appmenu-qt5

04: UWAGI LICENCYJNE
Kod TEA jest objęty licencją na licencji GPL V3 i pracuje jako domena publiczna. Media TEA (obrazy itp.), Wydania podręczników i tłumaczenia są wydawane. Uwaga dla współpracowników - prosimy o umieszczenie swoich tłumaczeń w domenie publicznej lub na licencji GPL.
====
