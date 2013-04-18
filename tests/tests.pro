VERSION = $$(QMLOG_VERSION)
TEMPLATE = app
QT -= gui

TARGET = logtest
INSTALLS = target tests

equals(QT_MAJOR_VERSION, 4): LIBS += -lqmlog
equals(QT_MAJOR_VERSION, 5): LIBS += -lqmlog-qt5
QMAKE_LIBDIR_FLAGS += -L../src

INCLUDEPATH += ../H

SOURCES = logtest.cpp

equals(QT_MAJOR_VERSION, 4) {
    tests.path = /usr/share/qmlog-tests
    tests.files = tests.xml
}
equals(QT_MAJOR_VERSION, 5) {
    tests.path = /usr/share/qmlog-qt5-tests
    tests.files = qt5/tests.xml
}

target.path = /usr/bin

QMAKE_CXXFLAGS  += -Wall -Werror
QMAKE_CXXFLAGS  += -Wno-psabi
