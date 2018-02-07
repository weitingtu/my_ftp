CC = gcc
CFLAGS = -O2
ifeq ($(MODE),rd)
	CFLAGS :=$(filter-out -O2,$(CFLAGS))
    CFLAGS += -g3 -gdwarf-4 -ggdb3 
endif
LDFLAGS = -lpthread

all: myftpclient myftpserver myftp.o myftpclient.o myftpserver.o

myftp.o : myftp.c myftp.h
	$(CC) -o $@ -c $< $(CFLAGS)

myftpserver.o: myftpserver.c myftp.h
	$(CC) -o $@ -c $< $(CFLAGS)

myftpclient.o: myftpclient.c myftp.h
	$(CC) -o $@ -c $< $(CFLAGS)

myftpserver: myftpserver.o myftp.o
	$(CC) -o $@ $^ $(LDFLAGS)

myftpclient: myftpclient.o myftp.o 
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -f myftpserver myftpclient myftp.o myftpclient.o myftpserver.o
