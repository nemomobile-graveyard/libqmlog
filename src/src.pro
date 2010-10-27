VERSION = 0.0.4
TEMPLATE=lib
QT -= gui

TARGET = qmlog
INSTALLS = target usr_include usr_include_qmlog prf

SOURCES += api2.cpp

target.path = $$(DESTDIR)/usr/lib

usr_include.path = $$(DESTDIR)/usr/include
usr_include.files = qmlog.h

usr_include_qmlog.path = $$(DESTDIR)/usr/include/qmlog
usr_include_qmlog.files = api2.h

prf.files = qmlog.prf
prf.path = $$(DESTDIR)/usr/share/qt4/mkspecs/features

QMAKE_CXXFLAGS  += -Wall -Werror
QMAKE_CXXFLAGS  += -Wno-psabi
