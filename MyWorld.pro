#-------------------------------------------------
#
# Project created by QtCreator 2021-12-11T23:15:51
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MyWorld
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        World.cpp \
    HashMap.cpp \
    Noise.cpp \
    Client.cpp \
    Matrix.cpp \
    Cube.cpp \
    LoginWindow/Login.cpp \
    LoginWindow/Register.cpp \
    LoginWindow/MainWindow.cpp \
    LoginWindow/Start.cpp \
    LoginWindow/CreateMap.cpp \
    LoginWindow/LoadMap.cpp \
    LoadingMap.cpp \
    RoomClient.cpp \
    GameMenu.cpp \
    LoginWindow/CreateRoom.cpp \
    LoginWindow/EnterRoom.cpp

HEADERS += \
        World.h \
    HashMap.h \
    Noise.h \
    Client.h \
    Matrix.h \
    Cube.h \
    Common.h \
    LoginWindow/Login.h \
    LoginWindow/Register.h \
    LoginWindow/MainWindow.h \
    LoginWindow/Start.h \
    LoginWindow/CreateMap.h \
    LoadingMap.h \
    LoginWindow/LoadMap.h \
    RoomClient.h \
    GameMenu.h \
    LoginWindow/CreateRoom.h \
    LoginWindow/EnterRoom.h

RESOURCES += \
    shader.qrc \
    texture.qrc \
    resource.qrc

LIBS += -lWs2_32

FORMS += \
    LoginWindow/Login.ui \
    LoginWindow/Register.ui \
    LoadingWidget.ui \
    LoginWindow/MainWindow.ui \
    LoginWindow/Start.ui \
    LoginWindow/CreateMap.ui \
    LoginWindow/LoadMap.ui \
    GameMenu.ui \
    LoginWindow/CreateRoom.ui \
    LoginWindow/EnterRoom.ui \
    World.ui

RC_FILE = myworld.rc


