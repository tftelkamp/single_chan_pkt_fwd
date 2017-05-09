# single_chan_pkt_fwd
# Single Channel LoRaWAN Gateway

CC=g++
CFLAGS=-c -Wall
LIBS=-lwiringPi

all: single_chan_pkt_fwd

single_chan_pkt_fwd: base64.o main.o
	$(CC) main.o base64.o $(LIBS) -o single_chan_pkt_fwd

main.o: main.cpp
	$(CC) $(CFLAGS) main.cpp

base64.o: base64.c
	$(CC) $(CFLAGS) base64.c

clean:
	rm *.o single_chan_pkt_fwd	
	
install : SCLoRaGWd
	cp SCLoRaGWd /usr/local/bin
	cp SCLoRaGWdService /etc/init.d/SCLoRaGWd
	chmod +x /etc/init.d/SCLoRaGWd
	update-rc.d SCLoRaGWd defaults
	service SCLoRaGWd start
	
uninstall:
	service SCLoRaGWd stop
	update-rc.d -f SCLoRaGWd remove
	rm /etc/init.d/SCLoRaGWd
	rm /usr/local/bin/SCLoRaGWd
	
SCLoRaGWd : main.cpp base64.c
	$(CC) -Wall main.cpp base64.c -o SCLoRaGWd $(LIBS) -DDAEMON