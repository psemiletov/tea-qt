#based on Ercole 'ercolinux' Carpanetto package
Name:          tea
Version:       44.1.0
Release:        %mkrel 2
Summary:       A powerful and easy-to-use Qt-based editor with many useful features for HTML and LaTeX editing
Group:         Editors
Vendor:        semiletov
Distribution:  semiletov
#Packager:      Ercole 'ercolinux' Carpanetto <ercole69@...>
Packager:      Peter Semiletov <semiletov@gmail.com>
URL:           http://semiletov.org/tea/
Source:        http://semiletov.org/tea/dloads/tea-%{version}.tar.bz2
License:       GPL V3
## AUTOBUILDREQ-BEGIN
BuildRequires: glibc-devel
BuildRequires: libgcc
#BuildRequires: libGL-devel
BuildRequires: libhunspell-devel
BuildRequires: libqt5-devel
BuildRequires: libstdc++6-devel
BuildRequires: libz-devel
## AUTOBUILDREQ-END
BuildRoot:     %{_tmppath}/%{name}-%{version}-root

%define debug_package %{nil}

## %debug_package

%description
TEA is a powerful and easy-to-use Qt-based editor with many useful features for HTML and LaTeX editing. It features a small footprint, a tabbed layout engine, support for multiple encodings, code snippets, templates, customizable hotkeys, an "open at cursor" function for HTML/LaTeX files and images, miscellaneous HTML tools, preview in external browser, string manipulation functions, Morse-code tools, bookmarks, syntax highlighting, and more.

%prep
%setup -q
#%patch0 -p1

%build
qmake PREFIX=%{_bindir}
%make

%install
[ "%{buildroot}" != / ] && rm -rf "%{buildroot}"
%makeinstall INSTALL_ROOT=%{buildroot}/ 

%clean
[ "%{buildroot}" != / ] && rm -rf "%{buildroot}"

%files
%defattr(-,root,root)
%{_bindir}/tea
#%doc AUTHORS COPYING
/usr/share/applications/tea.desktop
/usr/share/icons/hicolor/128x128/apps/tea-icon-v3-03.png
%changelog