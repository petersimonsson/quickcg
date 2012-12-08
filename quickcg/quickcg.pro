QT += core gui declarative xml network

TARGET = quickcg
TEMPLATE = app

LIBS += -lqjson

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
