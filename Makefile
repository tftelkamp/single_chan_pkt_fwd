# single_chan_pkt_fwd
# Single Channel LoRaWAN Gateway

CC=g++
CFLAGS=-Wall
LIBS=-lwiringPi

all: single_chan_pkt_fwd

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

single_chan_pkt_fwd: base64.o main.o
	$(CC) $^ $(LIBS) -o $@

clean:
	-rm *.o single_chan_pkt_fwd
