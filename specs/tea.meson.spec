Name:          tea
Version:       46.0.0
Release:       %mkrel 1
Summary:       A powerful Qt-based editor with useful functions for HTML and LaTeX editing
Group:         Editors
URL:           http://semiletov.org/tea
Source0:       http://semiletov.org/tea/dloads/tea-%{version}.tar.bz2

License:       GPLv3+

BuildRequires: libhunspell-devel
BuildRequires: libz-devel

BuildRequires: pkgconfig(Qt5Core)
BuildRequires: pkgconfig(Qt5Gui)
BuildRequires: pkgconfig(Qt5Network)
BuildRequires: pkgconfig(Qt5Widgets)
BuildRequires: pkgconfig(Qt5Xml)
BuildRequires: pkgconfig(Qt5Qml)
BuildRequires: pkgconfig(Qt5Quick)
BuildRequires: pkgconfig(Qt5PrintSupport)

BuildRequires: meson

%description
TEA is the lighthweight Qt-based editor with functions for HTML and LaTeX editing. 

It support for multiple encodings, code snippets, templates, plugins, user hotkeys, 
external scripts, string manipulation functions, bookmarks, syntax highlighting,
 and many more.

%prep
%setup -q

%build
%meson
%meson_build

%install
%meson_install -C build

%files
%{_bindir}/tea
%{_datadir}/applications/tea.desktop
%{_iconsdir}/hicolor/*/apps/tea.png
%{_iconsdir}/hicolor/scalable/apps/tea.svg
