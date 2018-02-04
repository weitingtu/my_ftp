TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
 
SOURCES += \
    server.c \
    client.c \
    MTserver.c \
    MTclient.c \
    MyFtpServer.c \
    MyFtpClient.c \
    cmd.c \

HEADERS += \
    cmd.h \
