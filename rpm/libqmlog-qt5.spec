Name:       libqmlog-qt5

# >> macros
%define _name qmlog-qt5
# << macros

Summary:    Generic logging library
Version:    0.10
Release:    1
Group:      System/System Control
License:    LGPLv2
URL:        http://meego.gitorious.org/meego-middleware/qmlog
Source0:    %{_name}-%{version}.tar.bz2
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
BuildRequires:  pkgconfig(Qt5Core)

%description
This package provides a library for writing log.

%package tests
Summary:    Testcases for qmlog library
Group:      Development/System
Requires:   %{name} = %{version}-%{release}
Requires:   testrunner-lite

%description tests
%{summary}.

%package devel
Summary:    Development package for %{name}
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}
Requires:   pkgconfig(Qt5Core)

%description devel
Provides header files for qmlog library.


%prep
%setup -q -n %{_name}-%{version}

# >> setup
# << setup

%build
# >> build pre
export QMLOG_VERSION=`head -n1 debian/changelog | sed "s/.*(\([^)+]*\).*/\1/"`
qmake
make
# << build pre



# >> build post
# << build post

%install
rm -rf %{buildroot}
# >> install pre
# << install pre

# >> install post
%qmake_install
install -d %{buildroot}/%{_datadir}/%{name}-tests/
mv %{buildroot}/%{_datadir}/%{_name}-tests/tests.xml %{buildroot}/%{_datadir}/%{name}-tests/tests.xml
# << install post


%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
# >> files
%doc COPYING debian/changelog debian/copyright
%{_libdir}/%{name}.so.*
# << files

%files tests
%defattr(-,root,root,-)
# >> files tests
%doc COPYING
%{_bindir}/qmlog-qt5-example
%{_datadir}/%{name}-tests/tests.xml
# << files tests

%files devel
%defattr(-,root,root,-)
# >> files devel
%doc COPYING
%{_includedir}/*
%{_libdir}/%{name}.so
%{_datadir}/qt5/mkspecs/features/qmlog-qt5.prf
# << files devel
