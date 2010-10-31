VERSION = 0.0.4
TEMPLATE=lib
QT -= gui

TARGET = qmlog
INSTALLS += target usr_include usr_include_libqmlog prf

SOURCES += api2.cpp

target.path = $$(DESTDIR)/usr/lib

usr_include.path = $$(DESTDIR)/usr/include
usr_include.files = qmlog

usr_include_libqmlog.path = $$(DESTDIR)/usr/include/libqmlog
usr_include_libqmlog.files = api2.h

prf.files = qmlog.prf
prf.path = $$(DESTDIR)/usr/share/qt4/mkspecs/features

QMAKE_CXXFLAGS  += -Wall -Werror
QMAKE_CXXFLAGS  += -Wno-psabi
