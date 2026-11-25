Name:       glacier-packagemanager
Summary:    Package Manager UI for Nemo
Version:    0.2.0
Release:    1
Group:      Applications/System
License:    LGPLv2.1
URL:        https://github.com/nemomobile-ux/glacier-packagemanager
Source0:    %{name}-%{version}.tar.bz2

Requires:   libglacierapp >= 0.8.0
Requires:   nemo-qml-plugin-controls-qt6

BuildRequires:  cmake
BuildRequires:  extra-cmake-modules
BuildRequires:  pkgconfig(Qt6Core)
BuildRequires:  pkgconfig(Qt6Gui)
BuildRequires:  pkgconfig(Qt6Qml)
BuildRequires:  pkgconfig(Qt6Quick)
BuildRequires:  pkgconfig(glacierapp) >= 0.8.0
BuildRequires:  qt6-tools-linguist
BuildRequires:  desktop-file-utils

%description
Package management UI for Nemo Mobile.

%prep
%setup -q -n %{name}-%{version}

%build
mkdir build
cd build
cmake \
	-DCMAKE_BUILD_TYPE=Release \
	-DCMAKE_INSTALL_PREFIX=%{_prefix} \
	-DCMAKE_VERBOSE_MAKEFILE=ON \
	..
cmake --build .

%install
cd build
rm -rf %{buildroot}
DESTDIR=%{buildroot} cmake --build . --target install
desktop-file-install --delete-original \
	--dir %{buildroot}%{_datadir}/applications \
	%{buildroot}%{_datadir}/applications/*.desktop

%files
%defattr(-,root,root,-)
%{_bindir}/glacier-packagemanager
%{_bindir}/glacier-packagemanager-daemon
%{_libdir}/glacier-packagemanager
%{_datadir}/glacier-packagemanager
%{_datadir}/applications/glacier-packagemanager.desktop
%{_datadir}/dbus-1/services/org.nemomobile.glacier.packagemanager.service
%{_datadir}/polkit-1/actions/org.nemomobile.glacier.packagemanager.policy
%{_libdir}/systemd/user/glacier-packagemanager-daemon.service
