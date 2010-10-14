VERSION = 0.0.4
TEMPLATE = app
QT -= gui

TARGET = logtest
INSTALLS = target tests

LIBS += -lqmlog
QMAKE_LIBDIR_FLAGS += -L../src

INCLUDEPATH += ../H

SOURCES = logtest.cpp

tests.path = /usr/share/qmlog-tests
tests.files = tests.xml

target.path = /usr/bin

QMAKE_CXXFLAGS  += -Wall -Werror
QMAKE_CXXFLAGS  += -Wno-psabi
