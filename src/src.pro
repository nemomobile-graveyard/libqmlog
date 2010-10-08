VERSION = 0.0.4
TEMPLATE=lib
QT -= gui

SOURCES += LoggerSettings.cpp
SOURCES += LoggerDev.cpp
SOURCES += FileLoggerDev.cpp
SOURCES += StdErrLoggerDev.cpp
SOURCES += StdOutLoggerDev.cpp
SOURCES += SysLogDev.cpp
SOURCES += log_t.cpp

INCLUDEPATH += ../H

TARGET = qmlog
target.path = /usr/lib

devheaders.files += LoggerSettings.h
devheaders.files += LoggerDev.h
devheaders.files += FileLoggerDev.h
devheaders.files += StdErrLoggerDev.h
devheaders.files += StdOutLoggerDev.h
devheaders.files += SysLogDev.h
devheaders.files += log_t.h
devheaders.files += log-interface.h
devheaders.files += log-declarations.h
devheaders.files += log
devheaders.path  = /usr/include/qm

prf.files = iodata.prf
prf.path = /usr/share/qt4/mkspecs/features

INSTALLS = target devheaders prf
