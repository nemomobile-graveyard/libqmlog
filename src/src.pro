VERSION = 0.$$(QMLOG_VERSION)
TEMPLATE=lib
QT -= gui

equals(QT_MAJOR_VERSION, 4): TARGET = qmlog
equals(QT_MAJOR_VERSION, 5): TARGET = qmlog-qt5

INSTALLS += target usr_include usr_include_libqmlog prf
equals(QT_MAJOR_VERSION, 4): INSTALLS += old_header

SOURCES += api2.cpp

target.path = $$(DESTDIR)/usr/lib

usr_include.path = $$(DESTDIR)/usr/include
equals(QT_MAJOR_VERSION, 4): usr_include.files = qmlog
equals(QT_MAJOR_VERSION, 5): usr_include.files = qmlog-qt5

equals(QT_MAJOR_VERSION, 4): usr_include_libqmlog.path = $$(DESTDIR)/usr/include/libqmlog
equals(QT_MAJOR_VERSION, 5): usr_include_libqmlog.path = $$(DESTDIR)/usr/include/libqmlog-qt5
usr_include_libqmlog.files = api2.h

old_header.path = $$(DESTDIR)/usr/include/qm
old_header.files = log

equals(QT_MAJOR_VERSION, 4) {
    prf.files = qmlog.prf
    prf.path = $$(DESTDIR)/usr/share/qt4/mkspecs/features
}
equals(QT_MAJOR_VERSION, 5) {
    prf.files = qmlog-qt5.prf
    prf.path = $$(DESTDIR)/usr/share/qt5/mkspecs/features
}

QMAKE_CXXFLAGS  += -Wall -Werror
QMAKE_CXXFLAGS  += -Wno-psabi
