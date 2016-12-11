 /******************************************************************************
 *
 * Copyright (c) 2015 Thomas Telkamp
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Changes for creating a dual channel gateway with the Raspberry Pi+ LoRa(TM) Expansion Board
 * of Uputronics, see also: store.uputronics.com/index.php?route=product/product&product_id=68
 *******************************************************************************/

// Raspberry PI pin mapping
// Pin number in this global_conf.json are Wiring Pi number (wPi colunm)
// issue a `gpio readall` on PI command line to see mapping
// +-----+-----+---------+------+---+---Pi 3---+---+------+---------+-----+-----+
// | BCM | wPi |   Name  | Mode | V | Physical | V | Mode | Name    | wPi | BCM |
// +-----+-----+---------+------+---+----++----+---+------+---------+-----+-----+
// |     |     |    3.3v |      |   |  1 || 2  |   |      | 5v      |     |     |
// |   2 |   8 |   SDA.1 |   IN | 1 |  3 || 4  |   |      | 5V      |     |     |
// |   3 |   9 |   SCL.1 |   IN | 1 |  5 || 6  |   |      | 0v      |     |     |
// |   4 |   7 | GPIO. 7 |   IN | 1 |  7 || 8  | 1 | ALT5 | TxD     | 15  | 14  |
// |     |     |      0v |      |   |  9 || 10 | 1 | ALT5 | RxD     | 16  | 15  |
// |  17 |   0 | GPIO. 0 |  OUT | 0 | 11 || 12 | 0 | IN   | GPIO. 1 | 1   | 18  |
// |  27 |   2 | GPIO. 2 |   IN | 0 | 13 || 14 |   |      | 0v      |     |     |
// |  22 |   3 | GPIO. 3 |   IN | 0 | 15 || 16 | 0 | IN   | GPIO. 4 | 4   | 23  |
// |     |     |    3.3v |      |   | 17 || 18 | 1 | IN   | GPIO. 5 | 5   | 24  |
// |  10 |  12 |    MOSI | ALT0 | 0 | 19 || 20 |   |      | 0v      |     |     |
// |   9 |  13 |    MISO | ALT0 | 0 | 21 || 22 | 0 | IN   | GPIO. 6 | 6   | 25  |
// |  11 |  14 |    SCLK | ALT0 | 0 | 23 || 24 | 1 | OUT  | CE0     | 10  | 8   |
// |     |     |      0v |      |   | 25 || 26 | 1 | OUT  | CE1     | 11  | 7   |
// |   0 |  30 |   SDA.0 |   IN | 1 | 27 || 28 | 1 | IN   | SCL.0   | 31  | 1   |
// |   5 |  21 | GPIO.21 |  OUT | 0 | 29 || 30 |   |      | 0v      |     |     |
// |   6 |  22 | GPIO.22 |  OUT | 0 | 31 || 32 | 1 | IN   | GPIO.26 | 26  | 12  |
// |  13 |  23 | GPIO.23 |  OUT | 0 | 33 || 34 |   |      | 0v      |     |     |
// |  19 |  24 | GPIO.24 |   IN | 0 | 35 || 36 | 0 | IN   | GPIO.27 | 27  | 16  |
// |  26 |  25 | GPIO.25 |   IN | 0 | 37 || 38 | 0 | IN   | GPIO.28 | 28  | 20  |
// |     |     |      0v |      |   | 39 || 40 | 0 | OUT  | GPIO.29 | 29  | 21  |
// +-----+-----+---------+------+---+----++----+---+------+---------+-----+-----+
// | BCM | wPi |   Name  | Mode | V | Physical | V | Mode | Name    | wPi | BCM |
// +-----+-----+---------+------+---+---Pi 3---+---+------+---------+-----+-----+
// For Uputronics Raspberry Pi+ LoRa(TM) Expansion Board
// pins configuration in file global_conf.json
//
//
//  "pin_nss": 10,
//  "pin_dio0": 6,
//  "pin_nss2": 11,
//  "pin_dio0_2": 27,
//  "pin_rst": 0,
//  "pin_NetworkLED": 22,
//  "pin_InternetLED": 23,
//  "pin_ActivityLED_0": 21,
//  "pin_ActivityLED_1": 29,
//


#include "base64.h"

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <wiringPi.h>
#include <wiringPiSPI.h>

#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netdb.h>

#include <cstdlib>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

static const int CHANNEL = 0;

using namespace std;

using namespace rapidjson;

#define BASE64_MAX_LENGTH 341

static const int SPI_CHANNEL = 0;
static const int SPI_CHANNEL_2 = 1;

bool sx1272 = true;
typedef unsigned char byte;

struct sockaddr_in si_other;
int s;
int slen = sizeof(si_other);
struct ifreq ifr;

uint32_t cp_nb_rx_rcv;
uint32_t cp_nb_rx_ok;
uint32_t cp_nb_rx_ok_tot;
uint32_t cp_nb_rx_bad;
uint32_t cp_nb_rx_nocrc;
uint32_t cp_up_pkt_fwd;

typedef enum SpreadingFactors
{
    SF7 = 7,
    SF8,
    SF9,
    SF10,
    SF11,
    SF12
} SpreadingFactor_t;

typedef struct Server
{
    string address;
    uint16_t port;
    bool enabled;
} Server_t;

/*******************************************************************************
 *
 * Default values, configure them in global_conf.json
 *
 *******************************************************************************/

// uputronics - Raspberry connections
// Put them in global_conf.json
int ssPin = 0xff;
int dio0  = 0xff;
int ssPin_2 = 0xff;
int dio0_2  = 0xff;
int RST   = 0xff;
int Led1  = 0xff;
int NetworkLED    = 22;
int InternetLED   = 23;
int ActivityLED_0 = 21;
int ActivityLED_1 = 29; 

// Set location in global_conf.json
float lat =  0.0;
float lon =  0.0;
int   alt =  0;

/* Informal status fields */
char platform[24] ;    /* platform definition */
char email[40] ;       /* used for contact email */
char description[64] ; /* used for free form description */

// Set spreading factor (SF7 - SF12), &nd  center frequency
// Overwritten by the ones set in global_conf.json
SpreadingFactor_t sf = SF7;
uint16_t bw = 125;
uint32_t freq = 868100000; // in Mhz! (868.1)
uint32_t freq_2 = 868300000; // in Mhz! (868.3)

// Servers
vector<Server_t> servers;

// internet interface
char interface[6];     // Used to set the interface to communicate to the internet either eth0 or wlan0

// #############################################
// #############################################

#define REG_FIFO                    0x00
#define REG_FIFO_ADDR_PTR           0x0D
#define REG_FIFO_TX_BASE_AD         0x0E
#define REG_FIFO_RX_BASE_AD         0x0F
#define REG_RX_NB_BYTES             0x13
#define REG_OPMODE                  0x01
#define REG_FIFO_RX_CURRENT_ADDR    0x10
#define REG_IRQ_FLAGS               0x12
#define REG_DIO_MAPPING_1           0x40
#define REG_DIO_MAPPING_2           0x41
#define REG_MODEM_CONFIG            0x1D
#define REG_MODEM_CONFIG2           0x1E
#define REG_MODEM_CONFIG3           0x26
#define REG_SYMB_TIMEOUT_LSB        0x1F
#define REG_PKT_SNR_VALUE           0x19
#define REG_PAYLOAD_LENGTH          0x22
#define REG_IRQ_FLAGS_MASK          0x11
#define REG_MAX_PAYLOAD_LENGTH      0x23
#define REG_HOP_PERIOD              0x24
#define REG_SYNC_WORD               0x39
#define REG_VERSION                 0x42

#define SX72_MODE_RX_CONTINUOS      0x85
#define SX72_MODE_TX                0x83
#define SX72_MODE_SLEEP             0x80
#define SX72_MODE_STANDBY           0x81


#define PAYLOAD_LENGTH              0x40

// LOW NOISE AMPLIFIER
#define REG_LNA                     0x0C
#define LNA_MAX_GAIN                0x23
#define LNA_OFF_GAIN                0x00
#define LNA_LOW_GAIN                0x20

// CONF REG
#define REG1                        0x0A
#define REG2                        0x84

#define SX72_MC2_FSK                0x00
#define SX72_MC2_SF7                0x70
#define SX72_MC2_SF8                0x80
#define SX72_MC2_SF9                0x90
#define SX72_MC2_SF10               0xA0
#define SX72_MC2_SF11               0xB0
#define SX72_MC2_SF12               0xC0

#define SX72_MC1_LOW_DATA_RATE_OPTIMIZE  0x01 // mandated for SF11 and SF12

// FRF
#define REG_FRF_MSB              0x06
#define REG_FRF_MID              0x07
#define REG_FRF_LSB              0x08

#define FRF_MSB                  0xD9 // 868.1 Mhz
#define FRF_MID                  0x06
#define FRF_LSB                  0x66

#define BUFLEN 2048  //Max length of buffer

#define PROTOCOL_VERSION  1
#define PKT_PUSH_DATA 0
#define PKT_PUSH_ACK  1
#define PKT_PULL_DATA 2

#define PKT_PULL_RESP 3
#define PKT_PULL_ACK  4

#define TX_BUFF_SIZE    2048
#define STATUS_SIZE     1024

void LoadConfiguration(string filename);
void PrintConfiguration();

void Die(const char *s)
{
  perror(s);
  exit(1);
}

void SelectReceiver(byte CE)
{
   if (CE == 0)
   {
     digitalWrite(ssPin, LOW);
   } else {
     digitalWrite(ssPin_2, LOW);
   }
}  

void UnselectReceiver(byte CE)
{
    if (CE == 0)
    {
      digitalWrite(ssPin, HIGH);
    } else {
      digitalWrite(ssPin_2, HIGH);
    }   
}

uint8_t ReadRegister(uint8_t addr, byte CE)
{
    uint8_t spibuf[2];

    SelectReceiver(CE);
    spibuf[0] = addr & 0x7F;
    spibuf[1] = 0x00;
    wiringPiSPIDataRW(CE, spibuf, 2);
    UnselectReceiver(CE);

    return spibuf[1];
}

void WriteRegister(uint8_t addr, uint8_t value, byte CE)
{
    uint8_t spibuf[2];
                           
    SelectReceiver(CE);
    spibuf[0] = addr | 0x80;
    spibuf[1] = value;
    wiringPiSPIDataRW(CE, spibuf, 2);

    UnselectReceiver(CE);    
}

bool ReceivePkt(char* payload, uint8_t* p_length, byte CE)
{
  // clear rxDone
  WriteRegister(REG_IRQ_FLAGS, 0x40, CE);

  int irqflags = ReadRegister(REG_IRQ_FLAGS, CE);

  cp_nb_rx_rcv++;

  //  payload crc: 0x20
  if((irqflags & 0x20) == 0x20) {
    printf("CRC error\n");
    WriteRegister(REG_IRQ_FLAGS, 0x20, CE);
    return false;

  } else {
    cp_nb_rx_ok++;
    cp_nb_rx_ok_tot++;

    uint8_t currentAddr = ReadRegister(REG_FIFO_RX_CURRENT_ADDR, CE);
    uint8_t receivedCount = ReadRegister(REG_RX_NB_BYTES, CE);
    *p_length = receivedCount;

    WriteRegister(REG_FIFO_ADDR_PTR, currentAddr, CE);

    for(int i = 0; i < receivedCount; i++) {
      payload[i] = ReadRegister(REG_FIFO, CE);
    }
  }
  return true;
}

char * PinName(int pin, char * buff) {
  strcpy(buff, "unused");
  if (pin != 0xff) {
    sprintf(buff, "%d", pin);
  }
  return buff;
}

void SetupLoRa(byte CE)
{
  char buff[16];
  if (CE == 0) {
    printf("Trying to detect module CE0 with ");
    printf("NSS=%s "  , PinName(ssPin, buff));
    printf("DIO0=%s " , PinName(dio0 , buff));
    printf("Reset=%s ", PinName(RST  , buff));
    printf("Led1=%s\n", PinName(Led1 , buff));
  } else {
    printf("Trying to detect module CE1 with ");
    printf("NSS=%s "  , PinName(ssPin_2, buff));
    printf("DIO0=%s " , PinName(dio0_2 , buff));
    printf("Reset=%s ", PinName(RST  , buff));
    printf("Led1=%s\n", PinName(Led1 , buff));
  }
  
  // check basic 
  if (ssPin == 0xff || dio0 == 0xff) {
    Die("Bad pin configuration ssPin and dio0 need at least to be defined");
  }


  uint8_t version = ReadRegister(REG_VERSION, CE);

  if (version == 0x22) {
    // sx1272
    printf("SX1272 detected, starting.\n");
    sx1272 = true;
  } else {
    // sx1276?
    version = ReadRegister(REG_VERSION, CE);
    if (version == 0x12) {
      // sx1276
      if (CE == 0)
      {
        printf("SX1276 detected on CE0, starting.\n");
      } else {
        printf("SX1276 detected on CE1, starting.\n");
      }
      sx1272 = false;
    } else {
      printf("Transceiver version 0x%02X\n", version);
      Die("Unrecognized transceiver");
    }
  }

  WriteRegister(REG_OPMODE, SX72_MODE_SLEEP, CE);

  // set frequency
  uint64_t frf;
  if (CE == 0)
  {
    frf = ((uint64_t)freq << 19) / 32000000;
  } else {
    frf = ((uint64_t)freq_2 << 19) / 32000000;
  }   
  WriteRegister(REG_FRF_MSB, (uint8_t)(frf >> 16), CE );
  WriteRegister(REG_FRF_MID, (uint8_t)(frf >> 8), CE );
  WriteRegister(REG_FRF_LSB, (uint8_t)(frf >> 0), CE );

  WriteRegister(REG_SYNC_WORD, 0x34, CE); // LoRaWAN public sync word

  if (sx1272) {
    if (sf == SF11 || sf == SF12) {
      WriteRegister(REG_MODEM_CONFIG, 0x0B, CE);
    } else {
      WriteRegister(REG_MODEM_CONFIG, 0x0A, CE);
    }
    WriteRegister(REG_MODEM_CONFIG2, (sf << 4) | 0x04, CE);
  } else {
    if (sf == SF11 || sf == SF12) {
      WriteRegister(REG_MODEM_CONFIG3, 0x0C, CE);
    } else {
      WriteRegister(REG_MODEM_CONFIG3, 0x04, CE);
    }
    WriteRegister(REG_MODEM_CONFIG, 0x72, CE);
    WriteRegister(REG_MODEM_CONFIG2, (sf << 4) | 0x04, CE);
  }

  if (sf == SF10 || sf == SF11 || sf == SF12) {
    WriteRegister(REG_SYMB_TIMEOUT_LSB, 0x05, CE);
  } else {
    WriteRegister(REG_SYMB_TIMEOUT_LSB, 0x08, CE);
  }
  WriteRegister(REG_MAX_PAYLOAD_LENGTH, 0x80, CE);
  WriteRegister(REG_PAYLOAD_LENGTH, PAYLOAD_LENGTH, CE);
  WriteRegister(REG_HOP_PERIOD, 0xFF, CE);
  WriteRegister(REG_FIFO_ADDR_PTR, ReadRegister(REG_FIFO_RX_BASE_AD, CE), CE);

  // Set Continous Receive Mode
  WriteRegister(REG_LNA, LNA_MAX_GAIN, CE);  // max lna gain
  WriteRegister(REG_OPMODE, SX72_MODE_RX_CONTINUOS, CE);
}

void SolveHostname(const char* p_hostname, uint16_t port, struct sockaddr_in* p_sin)
{
  struct addrinfo hints;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_protocol = IPPROTO_UDP;

  char service[6] = { '\0' };
  snprintf(service, 6, "%hu", port);

  struct addrinfo* p_result = NULL;

  // Resolve the domain name into a list of addresses
  int error = getaddrinfo(p_hostname, service, &hints, &p_result);
  if (error != 0) {
      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
      exit(EXIT_FAILURE);
  }

  // Loop over all returned results
  for (struct addrinfo* p_rp = p_result; p_rp != NULL; p_rp = p_rp->ai_next) {
    struct sockaddr_in* p_saddr = (struct sockaddr_in*)p_rp->ai_addr;
    //printf("%s solved to %s\n", p_hostname, inet_ntoa(p_saddr->sin_addr));
    p_sin->sin_addr = p_saddr->sin_addr;
  }

  freeaddrinfo(p_result);
}

void SendUdp(char *msg, int length)
{
  for (vector<Server_t>::iterator it = servers.begin(); it != servers.end(); ++it) {
    if (it->enabled) {
      si_other.sin_port = htons(it->port);

      SolveHostname(it->address.c_str(), it->port, &si_other);
      if (sendto(s, (char *)msg, length, 0 , (struct sockaddr *) &si_other, slen)==-1) {
        Die("sendto()");
      }
    }
  }
}

void SendStat()
{
  static char status_report[STATUS_SIZE]; /* status report as a JSON object */
  char stat_timestamp[24];

  int stat_index = 0;

  digitalWrite(InternetLED, HIGH);
  /* pre-fill the data buffer with fixed fields */
  status_report[0] = PROTOCOL_VERSION;
  status_report[3] = PKT_PUSH_DATA;

  status_report[4] = (unsigned char)ifr.ifr_hwaddr.sa_data[0];
  status_report[5] = (unsigned char)ifr.ifr_hwaddr.sa_data[1];
  status_report[6] = (unsigned char)ifr.ifr_hwaddr.sa_data[2];
  status_report[7] = 0xFF;
  status_report[8] = 0xFF;
  status_report[9] = (unsigned char)ifr.ifr_hwaddr.sa_data[3];
  status_report[10] = (unsigned char)ifr.ifr_hwaddr.sa_data[4];
  status_report[11] = (unsigned char)ifr.ifr_hwaddr.sa_data[5];

  /* start composing datagram with the header */
  uint8_t token_h = (uint8_t)rand(); /* random token */
  uint8_t token_l = (uint8_t)rand(); /* random token */
  status_report[1] = token_h;
  status_report[2] = token_l;
  stat_index = 12; /* 12-byte header */

  /* get timestamp for statistics */
  time_t t = time(NULL);
  strftime(stat_timestamp, sizeof stat_timestamp, "%F %T %Z", gmtime(&t));

  // Build JSON object.
  StringBuffer sb;
  Writer<StringBuffer> writer(sb);
  writer.StartObject();
  writer.String("stat");
  writer.StartObject();
  writer.String("time");
  writer.String(stat_timestamp);
  writer.String("lati");
  writer.Double(lat);
  writer.String("long");
  writer.Double(lon);
  writer.String("alti");
  writer.Int(alt);
  writer.String("rxnb");
  writer.Uint(cp_nb_rx_rcv);
  writer.String("rxok");
  writer.Uint(cp_nb_rx_ok);
  writer.String("rxfw");
  writer.Uint(cp_up_pkt_fwd);
  writer.String("ackr");
  writer.Double(0);
  writer.String("dwnb");
  writer.Uint(0);
  writer.String("txnb");
  writer.Uint(0);
  writer.String("pfrm");
  writer.String(platform);
  writer.String("mail");
  writer.String(email);
  writer.String("desc");
  writer.String(description);
  writer.EndObject();
  writer.EndObject();

  string json = sb.GetString();
  //printf("stat update: %s\n", json.c_str());
  printf("stat update: %s", stat_timestamp);
  if (cp_nb_rx_ok_tot==0) {
    printf(" no packet received yet\n");
  } else {
    printf(" %u packet%sreceived\n", cp_nb_rx_ok_tot, cp_nb_rx_ok_tot>1?"s ":" ");
  }

  // Build and send message.
  memcpy(status_report + 12, json.c_str(), json.size());
  SendUdp(status_report, stat_index + json.size());
  digitalWrite(InternetLED, LOW);
}

bool Receivepacket(byte CE)
{
  long int SNR;
  int rssicorr, dio_port;
  bool ret = false;

  if (CE == 0)
    {
        dio_port = dio0;
    } else {
        dio_port = dio0_2;
    }  

  if (digitalRead(dio_port) == 1) {
    char message[256];
    uint8_t length = 0;
    if (ReceivePkt(message, &length, CE)) {
      // OK got one
      ret = true;

      uint8_t value = ReadRegister(REG_PKT_SNR_VALUE, CE);

      if (CE == 0)
            {
              digitalWrite(ActivityLED_0, HIGH);
            } else {
              digitalWrite(ActivityLED_1, HIGH);
           }  

      if (value & 0x80) { // The SNR sign bit is 1
        // Invert and divide by 4
        value = ((~value + 1) & 0xFF) >> 2;
        SNR = -value;
      } else {
        // Divide by 4
        SNR = ( value & 0xFF ) >> 2;
      }

      rssicorr = sx1272 ? 139 : 157;

      printf("CE%i Packet RSSI: %d, ", CE, ReadRegister(0x1A, CE) - rssicorr);
      printf("RSSI: %d, ", ReadRegister(0x1B,CE) - rssicorr);
      printf("SNR: %li, ", SNR);
      printf("Length: %hhu Message:'", length);
      for (int i=0; i<length; i++) {
        char c = (char) message[i];
        printf("%c",isprint(c)?c:'.');
      }
      printf("'\n");

      char buff_up[TX_BUFF_SIZE]; /* buffer to compose the upstream packet */
      int buff_index = 0;

      /* gateway <-> MAC protocol variables */
      //static uint32_t net_mac_h; /* Most Significant Nibble, network order */
      //static uint32_t net_mac_l; /* Least Significant Nibble, network order */

      /* pre-fill the data buffer with fixed fields */
      buff_up[0] = PROTOCOL_VERSION;
      buff_up[3] = PKT_PUSH_DATA;

      /* process some of the configuration variables */
      //net_mac_h = htonl((uint32_t)(0xFFFFFFFF & (lgwm>>32)));
      //net_mac_l = htonl((uint32_t)(0xFFFFFFFF &  lgwm  ));
      //*(uint32_t *)(buff_up + 4) = net_mac_h; 
      //*(uint32_t *)(buff_up + 8) = net_mac_l;

      buff_up[4] = (uint8_t)ifr.ifr_hwaddr.sa_data[0];
      buff_up[5] = (uint8_t)ifr.ifr_hwaddr.sa_data[1];
      buff_up[6] = (uint8_t)ifr.ifr_hwaddr.sa_data[2]; 
      buff_up[7] = 0xFF;
      buff_up[8] = 0xFF;
      buff_up[9] = (uint8_t)ifr.ifr_hwaddr.sa_data[3];
      buff_up[10] = (uint8_t)ifr.ifr_hwaddr.sa_data[4];
      buff_up[11] = (uint8_t)ifr.ifr_hwaddr.sa_data[5];

      /* start composing datagram with the header */
      uint8_t token_h = (uint8_t)rand(); /* random token */
      uint8_t token_l = (uint8_t)rand(); /* random token */
      buff_up[1] = token_h;
      buff_up[2] = token_l;
      buff_index = 12; /* 12-byte header */

      // TODO: tmst can jump is time is (re)set, not good.
      struct timeval now;
      gettimeofday(&now, NULL);
      uint32_t tmst = (uint32_t)(now.tv_sec * 1000000 + now.tv_usec);

      // Encode payload.
      char b64[BASE64_MAX_LENGTH];
      bin_to_b64((uint8_t*)message, length, b64, BASE64_MAX_LENGTH);

      // Build JSON object.
      StringBuffer sb;
      Writer<StringBuffer> writer(sb);
      writer.StartObject();
      writer.String("rxpk");
      writer.StartArray();
      writer.StartObject();
      writer.String("tmst");
      writer.Uint(tmst);
      writer.String("freq");
      if (CE == 0) {
        writer.Double((double)freq / 1000000);
        writer.String("chan");
        writer.Uint(0);
      } else {
        writer.Double((double)freq_2 / 1000000);
        writer.String("chan");
        writer.Uint(1);
      }
      writer.String("rfch");
      writer.Uint(0);
      writer.String("stat");
      writer.Uint(1);
      writer.String("modu");
      writer.String("LORA");
      writer.String("datr");
      char datr[] = "SFxxBWxxx";
      snprintf(datr, strlen(datr) + 1, "SF%hhuBW%hu", sf, bw);
      writer.String(datr);
      writer.String("codr");
      writer.String("4/5");
      writer.String("rssi");
      writer.Int(ReadRegister(0x1A, CE) - rssicorr);
      writer.String("lsnr");
      writer.Double(SNR); // %li.
      writer.String("size");
      writer.Uint(length);
      writer.String("data");
      writer.String(b64);
      writer.EndObject();
      writer.EndArray();
      writer.EndObject();

      string json = sb.GetString();
      printf("rxpk update: %s\n", json.c_str());

      // Build and send message.
      memcpy(buff_up + 12, json.c_str(), json.size());
      SendUdp(buff_up, buff_index + json.size());

      fflush(stdout);
    }
  }
  return ret;
}

int main()
{
  struct timeval nowtime;
  uint32_t lasttime;
  unsigned int led0_timer,led1_timer;

  LoadConfiguration("global_conf.json");
  PrintConfiguration();

  // Init WiringPI
  wiringPiSetup() ;
  pinMode(ssPin, OUTPUT);
  pinMode(ssPin_2, OUTPUT);
  pinMode(dio0, INPUT);
  pinMode(dio0_2, INPUT);
  pinMode(RST, OUTPUT);
  pinMode(NetworkLED, OUTPUT);
  pinMode(ActivityLED_0, OUTPUT);
  pinMode(ActivityLED_1, OUTPUT);
  pinMode(InternetLED, OUTPUT);

  // Init SPI
  wiringPiSPISetup(SPI_CHANNEL, 500000);
  wiringPiSPISetup(SPI_CHANNEL_2, 500000);

  // Setup LORA
  digitalWrite(RST, HIGH);
  delay(100);
  digitalWrite(RST, LOW);
  delay(100);   
  SetupLoRa(0);
  SetupLoRa(1);

  // Prepare Socket connection
  while ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
     digitalWrite(NetworkLED, 1);
     printf("No socket connection possible yet. Retrying in 10 seconds...\n");
     delay(10000);
     digitalWrite(NetworkLED, 0);
  }

  memset((char *) &si_other, 0, sizeof(si_other));
  si_other.sin_family = AF_INET;

  ifr.ifr_addr.sa_family = AF_INET;
  strncpy(ifr.ifr_name, interface, IFNAMSIZ-1);  // use configured network interface eth0 or wlan0
  ioctl(s, SIOCGIFHWADDR, &ifr);

  // ID based on MAC Adddress of interface
  printf( "Gateway ID: %.2x:%.2x:%.2x:ff:ff:%.2x:%.2x:%.2x\n",
              (uint8_t)ifr.ifr_hwaddr.sa_data[0],
              (uint8_t)ifr.ifr_hwaddr.sa_data[1],
              (uint8_t)ifr.ifr_hwaddr.sa_data[2],
              (uint8_t)ifr.ifr_hwaddr.sa_data[3],
              (uint8_t)ifr.ifr_hwaddr.sa_data[4],
              (uint8_t)ifr.ifr_hwaddr.sa_data[5]
  );

  printf("Listening at SF%i on %.6lf Mhz.\n", sf,(double)freq/1000000);
  printf("Listening at SF%i on %.6lf Mhz.\n", sf,(double)freq_2/1000000);
  printf("-----------------------------------\n");

  while(1) {

    // Packet received ?
    if (Receivepacket(0)) {
      // Led ON
      if (ActivityLED_0 != 0xff) {
        digitalWrite(ActivityLED_0, 1);
      }

      // start our Led blink timer, LED as been lit in Receivepacket
      led0_timer=millis();
    }
    if (Receivepacket(1)) {
      // Led ON
      if (ActivityLED_1 != 0xff) {
        digitalWrite(ActivityLED_1, 1);
      }

      // start our Led blink timer, LED as been lit in Receivepacket
      led1_timer=millis();
    }

    gettimeofday(&nowtime, NULL);
    uint32_t nowseconds = (uint32_t)(nowtime.tv_sec);
    if (nowseconds - lasttime >= 30) {
      lasttime = nowseconds;
      SendStat();
      cp_nb_rx_rcv = 0;
      cp_nb_rx_ok = 0;
      cp_up_pkt_fwd = 0;
    }

    // Led timer in progress ?
    if (led0_timer) {
      // Led timer expiration, Blink duration is 250ms
      if (millis() - led0_timer >= 250) {
        // Stop Led timer
        led0_timer = 0;

        // Led OFF
        if (ActivityLED_0 != 0xff) {
          digitalWrite(ActivityLED_0, 0);
        }
      }
    }
    if (led1_timer) {
      // Led timer expiration, Blink duration is 250ms
      if (millis() - led1_timer >= 250) {
        // Stop Led timer
        led1_timer = 0;

        // Led OFF
        if (ActivityLED_1 != 0xff) {
          digitalWrite(ActivityLED_1, 0);
        }
      }
    }


    // Let some time to the OS
    delay(1);
  }

  return (0);
}

void LoadConfiguration(string configurationFile)
{
  FILE* p_file = fopen(configurationFile.c_str(), "r");
  char buffer[65536];
  FileReadStream fs(p_file, buffer, sizeof(buffer));

  Document document;
  document.ParseStream(fs);

  for (Value::ConstMemberIterator fileIt = document.MemberBegin(); fileIt != document.MemberEnd(); ++fileIt) {
    string objectType(fileIt->name.GetString());
    if (objectType.compare("SX127x_conf") == 0) {
      const Value& sx127x_conf = fileIt->value;
      if (sx127x_conf.IsObject()) {
        for (Value::ConstMemberIterator confIt = sx127x_conf.MemberBegin(); confIt != sx127x_conf.MemberEnd(); ++confIt) {
          string key(confIt->name.GetString());
          if (key.compare("freq") == 0) {
            freq = confIt->value.GetUint();
          } else if (key.compare("freq_2") == 0) {
            freq_2 = (SpreadingFactor_t)confIt->value.GetUint();
          } else if (key.compare("spread_factor") == 0) {
            sf = (SpreadingFactor_t)confIt->value.GetUint();
          } else if (key.compare("pin_nss") == 0) {
            ssPin = confIt->value.GetUint();
          } else if (key.compare("pin_nss_2") == 0) {
            ssPin_2 = confIt->value.GetUint();
          } else if (key.compare("pin_dio0") == 0) {
            dio0 = confIt->value.GetUint();
          } else if (key.compare("pin_dio0_2") == 0) {
            dio0_2 = confIt->value.GetUint();
          } else if (key.compare("pin_rst") == 0) {
            RST = confIt->value.GetUint();
          } else if (key.compare("pin_NetworkLED") == 0) {
            NetworkLED = confIt->value.GetUint();
          } else if (key.compare("pin_InternetLED") == 0) {
            InternetLED = confIt->value.GetUint();
          } else if (key.compare("pin_ActivityLED_0") == 0) {
            ActivityLED_0 = confIt->value.GetUint();
          } else if (key.compare("pin_ActivityLED_1") == 0) {
            ActivityLED_1 = confIt->value.GetUint();
          }
        }
      }

    } else if (objectType.compare("gateway_conf") == 0) {

      const Value& gateway_conf = fileIt->value;
      if (gateway_conf.IsObject()) {
        for (Value::ConstMemberIterator confIt = gateway_conf.MemberBegin(); confIt != gateway_conf.MemberEnd(); ++confIt) {
          string memberType(confIt->name.GetString());
          if (memberType.compare("ref_latitude") == 0) {
            lat = confIt->value.GetDouble();
          } else if (memberType.compare("ref_longitude") == 0) {
            lon = confIt->value.GetDouble();
          } else if (memberType.compare("ref_altitude") == 0) {
            alt = confIt->value.GetUint(); 

          } else if (memberType.compare("name") == 0 && confIt->value.IsString()) {
            string str = confIt->value.GetString();
            strcpy(platform, str.length()<=24 ? str.c_str() : "name too long");
          } else if (memberType.compare("email") == 0 && confIt->value.IsString()) {
            string str = confIt->value.GetString();
            strcpy(email, str.length()<=40 ? str.c_str() : "email too long");
          } else if (memberType.compare("desc") == 0 && confIt->value.IsString()) {
            string str = confIt->value.GetString();
            strcpy(description, str.length()<=64 ? str.c_str() : "description is too long");
          } else if (memberType.compare("interface") == 0 && confIt->value.IsString()) {
            string str = confIt->value.GetString();
            strcpy(interface, str.length()<=6 ? str.c_str() : "interface too long");

          } else if (memberType.compare("servers") == 0) {
            const Value& serverConf = confIt->value;
            if (serverConf.IsObject()) {
              const Value& serverValue = serverConf;
              Server_t server;
              for (Value::ConstMemberIterator srvIt = serverValue.MemberBegin(); srvIt != serverValue.MemberEnd(); ++srvIt) {
                string key(srvIt->name.GetString());
                if (key.compare("address") == 0 && srvIt->value.IsString()) {
                  server.address = srvIt->value.GetString();
                } else if (key.compare("port") == 0 && srvIt->value.IsUint()) {
                  server.port = srvIt->value.GetUint();
                } else if (key.compare("enabled") == 0 && srvIt->value.IsBool()) {
                  server.enabled = srvIt->value.GetBool();
                }
              }
              servers.push_back(server);
            }
            else if (serverConf.IsArray()) {
              for (SizeType i = 0; i < serverConf.Size(); i++) {
                const Value& serverValue = serverConf[i];
                Server_t server;
                for (Value::ConstMemberIterator srvIt = serverValue.MemberBegin(); srvIt != serverValue.MemberEnd(); ++srvIt) {
                  string key(srvIt->name.GetString());
                  if (key.compare("address") == 0 && srvIt->value.IsString()) {
                    server.address = srvIt->value.GetString();
                  } else if (key.compare("port") == 0 && srvIt->value.IsUint()) {
                    server.port = srvIt->value.GetUint();
                  } else if (key.compare("enabled") == 0 && srvIt->value.IsBool()) {
                    server.enabled = srvIt->value.GetBool();
                  }
                }
                servers.push_back(server);
              }
            }
          }
        }
      }
    }
  }
}

void PrintConfiguration()
{
  for (vector<Server_t>::iterator it = servers.begin(); it != servers.end(); ++it) {
    printf("server: .address = %s; .port = %hu; .enable = %d\n", it->address.c_str(), it->port, it->enabled);
  }
  printf("Gateway Configuration\n");
  printf("  %s (%s)\n  %s\n", platform, email, description);
  printf("  Latitude=%.8f\n  Longitude=%.8f\n  Altitude=%d\n", lat,lon,alt);
  printf("  Interface: %s\n", interface);

}
