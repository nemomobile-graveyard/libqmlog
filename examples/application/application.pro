TEMPLATE = app
TARGET = qmlog-example

SOURCES += qmlog-example.cpp
INCLUDEPATH += ../../src/ ../../

QMAKE_CXXFLAGS = -Wall -Werror -Wno-psabi
