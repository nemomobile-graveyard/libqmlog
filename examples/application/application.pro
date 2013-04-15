TEMPLATE = app
TARGET = qmlog-example

SOURCES += qmlog-example.cpp
INCLUDEPATH += ../../src/ ../../

QMAKE_LIBDIR_FLAGS += -L../../src
LIBS += -lqmlog -lrt

target.path = $$(DESTDIR)/usr/bin

tests.path = /usr/share/qmlog-tests
tests.files = tests.xml

QMAKE_CXXFLAGS = -Wall -Werror -Wno-psabi

INSTALLS += target tests
