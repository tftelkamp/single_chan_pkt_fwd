Dual Channel LoRaWAN Gateway
==============================
This repository contains a proof-of-concept implementation of a dual
channel LoRaWAN gateway with the Raspberry Pi+ LoRa(TM) Expansion Board of
Uputronics. It is based on (fork) the Single Channel LoRaWan Gateway
by Thomas Telkamp.

It has been tested on the Raspberry Pi platform, using a Raspberry Pi+ 
LoRa(TM) Expansion Board with 2 SX1276 (HopeRF RFM95W) on board.

Part of the source has been copied from the Semtech Packet Forwarder 
(with permission).

Maintainer: Thomas Telkamp <thomas@telkamp.eu>

The code is for testing and development purposes only, and is not meant 
for production usage. 

I will continue update this code for our own purposes; Hans Boksem (hans.boksem@gmail.com)

Features
--------
- listen on configurable frequency and spreading factor
- SF7 to SF12
- status updates
- can forward to two servers
- Uses 2 channels (as the board has 2 RFM95's), default set to 868.1 and 868.3 frequencies
- Autostart features
- logging to logfile (default /var/log/lora_gateway)
- Uses Hat LED's for indicating INTERNET, CEO and CE1 Data activity

Not (yet) supported:
- PACKET_PUSH_ACK processing
- SF7BW250 modulation
- FSK modulation
- downstream messages (tx)

Preparation, setup and running
------------------------------

Prepare SD Card
- download raspbian jessie lite
- unpack zip
- write img file with win32DiskImager or alike

Prepare Pi
- Insert SD card
- Start raspberry pi
- figure out the IP address (look into the routers DHCP config or attach monitor and find out there)
- login to the rasp (un/pw pi/raspberry)
- change the password (with passwd)
- run raspi-config (sudo raspi-config)
- Expand the filesystem 
- Goto advanced config and select SPI
- Enable SPI interface
- Enable loading of SPI kernel module by default
- exit the raspi-config and reboot

Install dependencies
- update the pkg database (twice command: sudo apt-get update)
- install wiringpi (sudo apt-get install wiringpi)

Install dual_chan_pkt_fw
- Put the zip file on the pi (using winscp or alike) or use git to clone the repository (git clone https://github.com/bokse001/dual_chan_pkt_fwd)
- unzip the file
- cd into the dual_chan_pkt_fw folder
- change the main.cpp with your gateway parameters (Informal status fields; platform, email, description) and location (lat,lon,alt))
- compile the dual_chan_pkt_fwd (make)

Install autostart dual_chan_pkt_fw
- copy the lora_gateway.sh script to /etc/init.d (sudo cp lora_gateway.sh /etc/init.d)
- set execute permission on lora_gateway.sh (sudo chmod a+x /etc/init.d/lora_gateway.sh)
- set execute permission on lora_gw_startup.sh (sudo chmod a+x lora_gw_startup.sh)
- configure the autostart (sudo update-rc.d lora_gateway.sh defaults)

Starting the dual_chan_pkt_fwd
- to start the dual_chan_pkt_fwd reboot or start via the commandline (/etc/init.d/lora_gateway.sh restart)
- Remember; it takes about 10 seconds to start the gateway...

Check the log
- Logging is done within the file /var/log/lora_gateway
- Remember; it takes about 10 seconds to start the gateway...

Troubleshooting
- When after rebooting one of the LEDs on the Hat (INTERNET) is on continuosly this means there is something wrong with the network. Fix the network, login and restart the lora_gateway

Connections
-----------
It is a raspberry Pi hat so just pop it on an rasp 2 or 3

Configuration
-------------

Defaults:

- LoRa:   SF7 at 868.1 and 868.3 Mhz
- Server: 40.114.249.243 port 1700  (The Things Network: router.eu.thethings.network)

Edit source node (main.cpp) to change configuration (look for: "Configure these values!").
If you want to use wifi change the following line to represent you wifi device:
strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);

Please set location, email and description.

License
-------
The source files in this repository are made available under the Eclipse
Public License v1.0, except for the base64 implementation, that has been
copied from the Semtech Packet Forwader.
