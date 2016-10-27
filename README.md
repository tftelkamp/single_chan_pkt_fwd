Dual Channel LoRaWAN Gateway
==============================
This repository contains a proof-of-concept implementation of a dual
channel LoRaWAN gateway.

It has been tested on the Raspberry Pi platform, using a Semtech SX1272
transceiver (HopeRF RFM92W), and SX1276 (HopeRF RFM95W).

The code is for testing and development purposes only, and is not meant
for production usage.

Part of the source has been copied from the Semtech Packet Forwarder
(with permission).

Maintainer: Thomas Telkamp <thomas@telkamp.eu>

Was forked by @jlesech https://github.com/tftelkamp/single_chan_pkt_fwd to add json configuration file    
then forked by @hallard https://github.com/hallard/single_chan_pkt_fwd 
then forked by @bokse001 https://github.com/bokse001/dual_chan_pkt_fwd to add dual channel support, 
    configurable network interface and uputronics Raspberry Pi+ LoRa(TM) Expansion Board

Added new Features
------------------

- Added support for [Dragino Lora HAT][2] and [LoRasPi][1] (more to come) and uputronics Raspberry Pi+ LoRa(TM) Expansion Board
- pin definition are in config file
- Removed some configuration hard coded in source file and put them into global_conf.json
- renamed main.cpp to dual_chan_pkt_fwd.cpp
- added dual_chan_pkt_fwd.service for systemd (debian jessie minimal) start
- added `make install` and `make uninstall` into Makefile to install service
- added control for On board Led's if any (uputronics board CE0 and CE1 activity, Internet and Lan sensing leds)
- added configuration of the network interface (eth0/wlan0) and sensing network interface connectivity
- added a counter for packets received since last start

Raspberry PI pin mapping is as follow and pin number in file `global_conf.json` are WiringPi pin number (wPi colunm)

```
root@xxxx # gpio readall
 +-----+-----+---------+------+---+---Pi 3---+---+------+---------+-----+-----+
 | BCM | wPi |   Name  | Mode | V | Physical | V | Mode | Name    | wPi | BCM |
 +-----+-----+---------+------+---+----++----+---+------+---------+-----+-----+
 |     |     |    3.3v |      |   |  1 || 2  |   |      | 5v      |     |     |
 |   2 |   8 |   SDA.1 |   IN | 1 |  3 || 4  |   |      | 5V      |     |     |
 |   3 |   9 |   SCL.1 |   IN | 1 |  5 || 6  |   |      | 0v      |     |     |
 |   4 |   7 | GPIO. 7 |   IN | 1 |  7 || 8  | 1 | ALT5 | TxD     | 15  | 14  |
 |     |     |      0v |      |   |  9 || 10 | 1 | ALT5 | RxD     | 16  | 15  |
 |  17 |   0 | GPIO. 0 |  OUT | 0 | 11 || 12 | 0 | IN   | GPIO. 1 | 1   | 18  |
 |  27 |   2 | GPIO. 2 |   IN | 0 | 13 || 14 |   |      | 0v      |     |     |
 |  22 |   3 | GPIO. 3 |   IN | 0 | 15 || 16 | 0 | IN   | GPIO. 4 | 4   | 23  |
 |     |     |    3.3v |      |   | 17 || 18 | 1 | IN   | GPIO. 5 | 5   | 24  |
 |  10 |  12 |    MOSI | ALT0 | 0 | 19 || 20 |   |      | 0v      |     |     |
 |   9 |  13 |    MISO | ALT0 | 0 | 21 || 22 | 0 | IN   | GPIO. 6 | 6   | 25  |
 |  11 |  14 |    SCLK | ALT0 | 0 | 23 || 24 | 1 | OUT  | CE0     | 10  | 8   |
 |     |     |      0v |      |   | 25 || 26 | 1 | OUT  | CE1     | 11  | 7   |
 |   0 |  30 |   SDA.0 |   IN | 1 | 27 || 28 | 1 | IN   | SCL.0   | 31  | 1   |
 |   5 |  21 | GPIO.21 |  OUT | 0 | 29 || 30 |   |      | 0v      |     |     |
 |   6 |  22 | GPIO.22 |  OUT | 0 | 31 || 32 | 1 | IN   | GPIO.26 | 26  | 12  |
 |  13 |  23 | GPIO.23 |  OUT | 0 | 33 || 34 |   |      | 0v      |     |     |
 |  19 |  24 | GPIO.24 |   IN | 0 | 35 || 36 | 0 | IN   | GPIO.27 | 27  | 16  |
 |  26 |  25 | GPIO.25 |   IN | 0 | 37 || 38 | 0 | IN   | GPIO.28 | 28  | 20  |
 |     |     |      0v |      |   | 39 || 40 | 0 | OUT  | GPIO.29 | 29  | 21  |
 +-----+-----+---------+------+---+----++----+---+------+---------+-----+-----+
 | BCM | wPi |   Name  | Mode | V | Physical | V | Mode | Name    | wPi | BCM |
 +-----+-----+---------+------+---+---Pi 3---+---+------+---------+-----+-----+

```

* For [Dragino RPI Lora][2] HAT    
pins configuration in `global_conf.json`
```
  "pin_nss": 6,
  "pin_dio0": 7,
  "pin_rst": 0
```

* For [LoRasPi][1] Shield    
pins configuration in file `global_conf.json`

```
  "pin_nss": 8,
  "pin_dio0": 6,
  "pin_rst": 3,
  "pin_led1":4
```

* For Uputronics Raspberry Pi+ LoRa(TM) Expansion Board
pins configuration in file `global_conf.json`

```
  "pin_nss": 10,
  "pin_dio0": 6,
  "pin_nss2": 11,
  "pin_dio0_2": 27,
  "pin_rst": 0,
  "pin_NetworkLED": 22,
  "pin_InternetLED": 23,
  "pin_ActivityLED_0": 21,
  "pin_ActivityLED_1": 29,
```


Installation
------------

Install dependencies as indicated in original README.md below then

```shell
cd /home/pi
git clone https://github.com/bokse001/dual_chan_pkt_fwd
make
sudo make install
````

To start service, as root or sudo (should already be started at boot if you done make install and rebooted of course), stop service or look service status
```shell
systemctl start dual_chan_pkt_fwd
systemctl stop dual_chan_pkt_fwd
systemctl status dual_chan_pkt_fwd
````

To see gateway log in real time
```shell
sudo journalctl -f -u dual_chan_pkt_fwd
````

Configuration
-------------

Defaults:

- LoRa:   SF7 at 868.1 Mhz and 868.3 Mhz
- Server: 40.114.249.243, port 1700  (The Things Network)

Please configure the global_conf.json file with your settigns like network interface, location, email and description.


Pictures
--------

running daemon on Raspberry PI with uputronics Raspberry Pi+ LoRa(TM) Expansion Board    

<img src="https://raw.githubusercontent.com/bokse001/dual_chan_pkt_fwd/master/images/dual-channel-gw-ttn.jpg" alt="Dual Channel GW TTN">


running daemon on Raspberry PI with LoRasPI shield    

<img src="https://raw.githubusercontent.com/hallard/LoRasPI/master/images/LoRasPI-on-Pi.jpg" alt="LoRasPI plugged on PI">



**Original README.md below**

Features
--------

listen on configurable frequency and spreading factor
SF7 to SF12
status updates
can forward to two servers
Not (yet) supported:

PACKET_PUSH_ACK processing
SF7BW250 modulation
FSK modulation
downstream messages (tx)

Dependencies
------------

SPI needs to be enabled on the Raspberry Pi (use raspi-config)
WiringPi: a GPIO access library written in C for the BCM2835 used in the Raspberry Pi. sudo apt-get install wiringpi see http://wiringpi.com
Run packet forwarder as root

Connections
-----------

SX127x	Raspberry PI
3.3V	 3.3V (header pin #1)
GND	GND (pin #6)
MISO	MISO (pin #21)
MOSI	MOSI (pin #19) 
SCK	CLK (pin #23)
NSS	GPIO6 (pin #22)
DIO0	GPIO7 (pin #7)
RST	GPIO0 (pin #11)
Configuration

Defaults:
---------

LoRa: SF7 at 868.1 Mhz
Server: 54.229.214.112, port 1700 (The Things Network: croft.thethings.girovito.nl)
Edit source node (main.cpp) to change configuration (look for: "Configure these values!").

Please set location, email and description.

License
-------

The source files in this repository are made available under the Eclipse Public License v1.0, except:

base64 implementation, that has been copied from the Semtech Packet Forwarder;
RapidJSON, licensed under the MIT License.