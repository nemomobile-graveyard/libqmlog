%define _name qmlog
Name:     libqmlog
Version:  0.10
Release:  1
Summary:  Generic logging library
Group:    System/System Control
License:  LGPLv2
URL:      http://meego.gitorious.org/meego-middleware/libqmlog
Source0:  %{_name}-%{version}.tar.bz2
Patch0:   %{name}-0.0.9-librt.patch

BuildRequires: pkgconfig(QtCore) >= 4.5

%description
This package provides a library for writing log.

%package devel
Summary:  Development package for %{name}
Group:    Development/Libraries
Requires: pkgconfig(QtCore) >= 4.5
Requires: %{name} = %{version}-%{release}

%description devel
Provides header files for qmlog library.

%package tests
Summary:  Testcases for qmlog library
Group:    Development/System
Requires: testrunner-lite

%description tests
%{summary}.

%prep
%setup -q -n %{_name}-%{version}
%patch0 -p1

%build
export QMLOG_VERSION=`head -n1 debian/changelog | sed "s/.*(\([^)+]*\).*/\1/"`
qmake
make

%install
%qmake_install
install -d %{buildroot}/%{_datadir}/%{name}-tests/
mv %{buildroot}/%{_datadir}/%{_name}-tests/tests.xml %{buildroot}/%{_datadir}/%{name}-tests/tests.xml

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%doc COPYING debian/changelog debian/copyright
%{_libdir}/%{name}.so.*

%files devel
%defattr(-,root,root,-)
%doc COPYING
%{_includedir}/*
%{_libdir}/%{name}.so
%{_datadir}/qt4/mkspecs/features/qmlog.prf

%files tests
%defattr(-,root,root,-)
%doc COPYING
%{_bindir}/qmlog-example
%{_datadir}/%{name}-tests/tests.xml
