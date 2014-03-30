QT += core gui declarative xml network widgets

TARGET = quickcg
TEMPLATE = app

SOURCES += main.cpp\
    mainwindow.cpp \
    graphic.cpp \
    show.cpp \
    server.cpp \
    clientconnection.cpp

HEADERS += mainwindow.h \
    graphic.h \
    show.h \
    server.h \
    clientconnection.h

FORMS += mainwindow.ui
