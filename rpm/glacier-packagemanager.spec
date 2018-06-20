Name:       glacier-packagemanager
Summary:    Package Manager UI for Nemo
Version:    0.1
Release:    1
Group:      Applications/System
License:    LGPLv2.1
URL:        https://github.com/nemomobile-ux/glacier-packagemanager
Source0:    %{name}-%{version}.tar.bz2

Requires:   PackageKit-Qt5 >= 0.9.6

BuildRequires:  PackageKit-Qt5-devel
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5DBus)
BuildRequires:  pkgconfig(Qt5Quick)
BuildRequires:  pkgconfig(XPackageKit)
BuildRequires:  pkgconfig(glacierapp)
BuildRequires:  desktop-file-utils


%description
Package management UI for Nemo

%prep
%setup -q -n %{name}-%{version}

%build

%qmake5

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}

%qmake_install
desktop-file-install --delete-original       \
  --dir %{buildroot}%{_datadir}/applications             \
   %{buildroot}%{_datadir}/applications/*.desktop

%files
%defattr(-,root,root,-)
%{_bindir}/glacier-packagemanager
%{_datadir}/glacier-packagemanager
%{_datadir}/applications/glacier-packagemanager.desktop
%{_datadir}/mapplauncherd/privileges.d/glacier-packagemanager.privileges
