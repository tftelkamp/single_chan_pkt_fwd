# dual_chan_pkt_fwd
# Dual Channel LoRaWAN Gateway

CC = g++
CFLAGS = -std=c++11 -c -Wall -I include/
LIBS = -lwiringPi

all: dual_chan_pkt_fwd

dual_chan_pkt_fwd: base64.o dual_chan_pkt_fwd.o
	$(CC) dual_chan_pkt_fwd.o base64.o $(LIBS) -o dual_chan_pkt_fwd

dual_chan_pkt_fwd.o: dual_chan_pkt_fwd.cpp
	$(CC) $(CFLAGS) dual_chan_pkt_fwd.cpp

base64.o: base64.c
	$(CC) $(CFLAGS) base64.c

clean:
	rm *.o dual_chan_pkt_fwd

install:
	sudo cp -f ./dual_chan_pkt_fwd.service /lib/systemd/system/
	sudo systemctl enable dual_chan_pkt_fwd.service
	sudo systemctl daemon-reload
	sudo systemctl start dual_chan_pkt_fwd
	sudo systemctl status dual_chan_pkt_fwd -l

uninstall:
	sudo systemctl stop dual_chan_pkt_fwd
	sudo systemctl disable dual_chan_pkt_fwd.service
	sudo rm -f /lib/systemd/system/dual_chan_pkt_fwd.service 
