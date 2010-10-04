%define _name qmlog
Name:    libqmlog
Version: 0.0.5
Release: 1
Summary: Generic logging library
Group:   System/Libraries
License: LGPLv2
URL:     http://meego.gitorious.org/meego-middleware/libqmlog
Source0: %{_name}-%{version}.tar.bz2

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

%description tests
%{summary}.

%prep
%setup -q -n %{_name}-%{version}

%build
qmake
make

%install
make INSTALL_ROOT=%{buildroot} install
install -d %{buildroot}/%{_datadir}/%{name}-tests/
mv %{buildroot}/%{_datadir}/%{_name}-tests/tests.xml %{buildroot}/%{_datadir}/%{name}-tests/tests.xml

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%doc COPYING debian/changelog
%{_libdir}/%{name}.so.*

%files devel
%defattr(-,root,root,-)
%doc COPYING
%{_includedir}/qm/*
%{_libdir}/%{name}.so
%{_datadir}/qt4/mkspecs/features/qmlog.prf

%files tests
%defattr(-,root,root,-)
%doc COPYING
%{_datadir}/%{name}-tests/tests.xml
%{_bindir}/logtest
