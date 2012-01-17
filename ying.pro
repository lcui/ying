TEMPLATE = app
DESTDIR = ./bin

INCLUDEPATH += $$quote(.)

HEADERS += 	yingwin.h


SOURCES += 	yingwin.cpp
SOURCES += 	ying.cpp
SOURCES += 	gitengine.cpp
            
RESOURCES += ying.qrc

CONFIG += debug_and_release

win32 {
    RC_FILE = ying.rc
}

