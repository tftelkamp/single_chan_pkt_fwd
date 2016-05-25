Single Channel LoRaWAN Gateway
==============================
This repository contains a proof-of-concept implementation of a single
channel LoRaWAN gateway.

It has been tested on the Raspberry Pi platform, using a Semtech SX1272
transceiver (HopeRF RFM92W), and SX1276 (HopeRF RFM95W).

The code is for testing and development purposes only, and is not meant 
for production usage. 

Part of the source has been copied from the Semtech Packet Forwarder 
(with permission).

Maintainer: Thomas Telkamp <thomas@telkamp.eu>

Features
--------
- listen on configurable frequency and spreading factor
- SF7 to SF12
- status updates
- can forward to two servers

Not (yet) supported:
- PACKET_PUSH_ACK processing
- SF7BW250 modulation
- FSK modulation
- downstream messages (tx)

Dependencies
------------
- SPI needs to be enabled on the Raspberry Pi (use raspi-config)
- WiringPi: a GPIO access library written in C for the BCM2835 
  used in the Raspberry Pi.
  sudo apt-get install wiringpi
  see http://wiringpi.com
- Run packet forwarder as root

Building packages for Debian Distributions
------------------------------------------

If you are running a debian distribution such as raspbian on
your pi, you can now create a debian package to install on
multiple devices by running the following commands:

```bash
sudo apt-get install devscripts build-essential lintian
debuild
cd ..
sudo dpkg -i lorawan-single-packet-gateway*.deb
```

NOTE: You will need a GPG key installed on the pi you are
building from, and you must build on a device with the same
architecture as that to which you will deploy.

Connections
-----------
SX1272 - Raspberry

3.3V   - 3.3V (header pin #1) 
GND	   - GND (pin #6)
MISO   - MISO (pin #21)
MOSI   - MOSI (pin #19)
SCK    - CLK (pin #23)
NSS    - GPIO6 (pin #22)
DIO0   - GPIO7 (pin #7)
RST    - GPIO0 (pin #11)

Configuration
-------------

Defaults:

- LoRa:   SF7 at 868.1 Mhz
- Server: 54.229.214.112, port 1700  (The Things Network: croft.thethings.girovito.nl)

Edit source node (main.cpp) to change configuration (look for: "Configure these values!").

Please set location, email and description.

License
-------
The source files in this repository are made available under the Eclipse
Public License v1.0, except for the base64 implementation, that has been
copied from the Semtech Packet Forwader.
