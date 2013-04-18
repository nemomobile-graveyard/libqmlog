TEMPLATE = app
equals(QT_MAJOR_VERSION, 4): TARGET = qmlog-example
equals(QT_MAJOR_VERSION, 5): TARGET = qmlog-qt5-example

SOURCES += qmlog-example.cpp
INCLUDEPATH += ../../src/ ../../

QMAKE_LIBDIR_FLAGS += -L../../src
equals(QT_MAJOR_VERSION, 4): LIBS += -lqmlog -lrt
equals(QT_MAJOR_VERSION, 5): LIBS += -lqmlog-qt5 -lrt

target.path = $$(DESTDIR)/usr/bin

equals(QT_MAJOR_VERSION, 4) {
    tests.path = /usr/share/qmlog-tests
    tests.files = tests.xml
}
equals(QT_MAJOR_VERSION, 5) {
    tests.path = /usr/share/qmlog-qt5-tests
    tests.files = qt5/tests.xml
}

QMAKE_CXXFLAGS = -Wall -Werror -Wno-psabi

INSTALLS += target tests
