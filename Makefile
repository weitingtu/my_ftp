CC = gcc
CFLAG = -lpthread

all: server client MTserver MTclient MyFtpClient MyFtpServer cmd.o MyFtpClient.o

server: server.c
	$(CC) -o $@ $<

client: client.c
	$(CC) -o $@ $<

MTserver: MTserver.c
	$(CC) -o $@ $< $(CFLAG)

MTclient: MTclient.c
	$(CC) -o $@ $< $(CFLAG)

cmd.o : cmd.c cmd.h
	$(CC) -o $@ -c $< $(CFLAG)

MyFtpServer.o: MyFtpServer.c cmd.h
	$(CC) -o $@ -c $< $(CFLAG)

MyFtpServer: MyFtpServer.o cmd.o
	$(CC) -o $@ $^ $(CFLAG)

MyFtpClient.o: MyFtpClient.c cmd.h
	$(CC) -o $@ -c $< $(CFLAG)

MyFtpClient: MyFtpClient.o cmd.o 
	$(CC) -o $@ $^ $(CFLAG)

clean:
	rm server client MTserver MTclient MyFtpServer MyFtpClient cmd.o MyFtpClient.o
