TEMPLATE = app
DESTDIR = ./bin

INCLUDEPATH += $$quote(.)

HEADERS += 	yingwin.h
HEADERS += 	txt2html.h


SOURCES += 	yingwin.cpp
SOURCES += 	ying.cpp
SOURCES += 	gitengine.cpp
SOURCES += 	txt2html.cpp
            
RESOURCES += ying.qrc

CONFIG += debug_and_release

win32 {
    RC_FILE = ying.rc
}

