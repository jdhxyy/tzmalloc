TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        main.c \
    ../../bget.c \
    ../../tzmalloc.c \
    ../../lib/scunit-clang/scunit.c

INCLUDEPATH += ../../
INCLUDEPATH += ../../lib/scunit-clang

HEADERS += \
    ../../bget.h \
    ../../tzmalloc.h \
    ../../lib/scunit-clang/scunit.h
