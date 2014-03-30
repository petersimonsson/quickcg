QT += core gui network widgets

TARGET = quickcgclient
TEMPLATE = app

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
