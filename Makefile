# single_chan_pkt_fwd
# Single Channel LoRaWAN Gateway

CC=g++
CFLAGS=-c -Wall
LIBS=-lwiringPi -lpthread

all: single_chan_pkt_fwd

single_chan_pkt_fwd: base64.o main.o
	$(CC) main.o base64.o $(LIBS) -o single_chan_pkt_fwd

main.o: main.cpp
	$(CC) $(CFLAGS) main.cpp

base64.o: base64.c
	$(CC) $(CFLAGS) base64.c

clean:
	-rm *.o single_chan_pkt_fwd	

install:
	install -m 0755 single_chan_pkt_fwd $(DESTDIR)$(prefix)/sbin
