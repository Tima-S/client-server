CC = clang
FLAGS = -lpthread

all: server client

server: server.o crc16.o
	$(CC) server.o crc16.o -o server $(FLAGS)

client: client.o crc16.o
	$(CC) client.o crc16.o -o client $(FLAGS)

server.o: server.c
	$(CC) -c server.c

crc16.o: crc16.c
	$(CC) -c crc16.c

client.o: client.c
	$(CC) -c client.c

clean:
	rm -rf *.o server client

