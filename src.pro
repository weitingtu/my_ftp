TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
 
SOURCES += \
    server.c \
    client.c \
    MTserver.c \
    MTclient.c \
    myftpserver.c \
    myftpclient.c \
    myftp.c \

HEADERS += \
    myftp.h \
