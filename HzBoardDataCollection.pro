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


#LIBS +=  \
#    chartdir60.lib

LIBS += chartdir60.lib
