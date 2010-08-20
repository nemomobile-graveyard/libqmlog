TEMPLATE = app
QT -= Gui
TARGET = logtest
INSTALLS = target tests

LIBS += -lqmlog

QMAKE_LIBDIR_FLAGS += -L../src
INCLUDEPATH += ../src

SOURCES = logtest.cpp

tests.path = /usr/share/qmlog-tests
tests.files = tests.xml

target.path = /usr/bin
