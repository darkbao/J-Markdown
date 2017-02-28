#-------------------------------------------------
#
# Project created by QtCreator 2017-02-28T13:50:26
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets webkitwidgets

TARGET = J-Markdown
TEMPLATE = app
RESOURCES = markdown.qrc
RC_FILE = app.rc

SOURCES += main.cpp\
        mainwindow.cpp \
    CHtmlRender.cpp \
    CMarkdown.cpp

HEADERS  += mainwindow.h \
    CHtmlRender.h \
    CMarkdown.h

DISTFILES +=
