VERSION = 0.0.4
TEMPLATE=lib
QT -= gui

SOURCES = log.cpp

INCLUDEPATH += ../H

TARGET = qmlog
target.path = /usr/lib

devheaders.files = log-declarations.h log
devheaders.path  = /usr/include/qm

prf.files = iodata.prf
prf.path = /usr/share/qt4/mkspecs/features

INSTALLS = target devheaders prf

QMAKE_CXXFLAGS  += -Wall -Wno-psabi

