# -------------------------------------------------
# QT Project File
# -------------------------------------------------
TARGET = HzBoardDataCollection

TEMPLATE = app

QT += serialport

RC_ICONS = v.ico

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
} 

SOURCES += qchartviewer.cpp \
    HzBoardDataCollection.cpp

HEADERS += qchartviewer.h \
    HzBoardDataCollection.h

RESOURCES += HzBoardDataCollection.qrc

INCLUDEPATH += include

DEFINES += CHARTDIR_HIDE_OBSOLETE _CRT_SECURE_NO_WARNINGS

win32:LIBS += ../HzBoardDataDisplay/chartdirector/x64/chartdir60.lib
win32:QMAKE_POST_LINK = copy /Y ..\\HzBoardDataDisplay\\chartdirector\\x64\\chartdir60.dll $(DESTDIR)

