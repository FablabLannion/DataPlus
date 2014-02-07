/*********************************************************************
 * Data+ AquaMarium project
 * Fablab Lannion
 * http://fablab-lannion.org/wiki/index.php?title=AquaMarium
 *********************************************************************
 * Copyright: (c) 2013 Jérôme Labidurie
 * Licence:   GNU General Public Licence version 3
 * Email:     jerome.labidurie at gmail.com
 *********************************************************************
 * This file is part of AquaMarium.
 *
 * AquaMarium is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * AquaMarium is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with AquaMarium.  If not, see <http://www.gnu.org/licenses/>.
 *********************************************************************
 * Needed Librairies :
 *********************************************************************
 */
#include <EtherShield.h>
#include "time.h"
#include "network.h"

// Please modify the following lines. mac and ip have to be unique
// in your local area network. You can not have the same numbers in
// two devices:
// how did I get the mac addr? Translate the first 3 numbers into ascii is: TUX

static uint8_t mymac[6] = { 0x54,0x55,0x58,0x12,0x34,0x57 };


static uint8_t myip[4] = { 0,0,0,0 };
static uint8_t mynetmask[4] = { 0,0,0,0 };

// IP address of the host being queried to contact (IP of the first portion of the URL):
static uint8_t ntpsrvip[4] = { 192, 168, 1, 20 };
uint8_t clientPort = 123;
uint32_t timeLong = 0;
unsigned long lastTime = 0;
unsigned long curTime = 0;

// Default gateway. The ip address of your DSL router. It can be set to the same as
// websrvip the case where there is no default GW to access the
// web server (=web server is on the same lan as this host)
static uint8_t gwip[4] = { 0,0,0,0};

static uint8_t dnsip[4] = { 0,0,0,0 };
static uint8_t dhcpsvrip[4] = { 0,0,0,0 };

#define DHCPLED 13


#define BUFFER_SIZE 500
static uint8_t buf[BUFFER_SIZE+1];

EtherShield es=EtherShield();

// Programmable delay for flashing LED
uint16_t delayRate = 1000;

void setup() {

  Serial.begin(19200);
  Serial.println("DHCP Client test");
  pinMode( DHCPLED, OUTPUT);
  digitalWrite( DHCPLED, HIGH);    // Turn it off

  for( int i=0; i<6; i++ ) {
    Serial.print( mymac[i], HEX );
    Serial.print( i < 5 ? ":" : "" );
  }
  Serial.println();

  // Initialise SPI interface
  es.ES_enc28j60SpiInit();

  // initialize enc28j60
  Serial.println("Init ENC28J60");

  es.ES_enc28j60Init(mymac);


  Serial.println("Init done");

  Serial.print( "ENC28J60 version " );
  Serial.println( es.ES_enc28j60Revision(), HEX);
  if( es.ES_enc28j60Revision() <= 0 ) {
    Serial.println( "Failed to access ENC28J60");

    while(1);    // Just loop here
  }

   dhcpGet();
  ntpDate(ntpsrvip);
  lastTime = millis();

}


void loop(){
   char day[22];
   char clock[22];

   gmtime(timeLong - GETTIMEOFDAY_TO_NTP_OFFSET,day,clock);
   Serial.print( day );
   Serial.print( " " );
   Serial.println( clock );

  digitalWrite( DHCPLED, HIGH);
  delay(delayRate);
  digitalWrite(DHCPLED, LOW);
  delay(delayRate);

  // update current date
  curTime = millis();
  timeLong += (curTime - lastTime) / 1000;
  lastTime = curTime;

}



