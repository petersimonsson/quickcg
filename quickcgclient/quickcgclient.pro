QT += core gui network

TARGET = quickcgclient
TEMPLATE = app

LIBS += -lqjson

SOURCES += main.cpp\
        mainwindow.cpp \
    serverconnection.cpp \
    creategraphicdialog.cpp \
    graphicpropertiesdialog.cpp

HEADERS += mainwindow.h \
    serverconnection.h \
    creategraphicdialog.h \
    graphicpropertiesdialog.h

FORMS += mainwindow.ui \
    creategraphicdialog.ui \
    graphicpropertiesdialog.ui
