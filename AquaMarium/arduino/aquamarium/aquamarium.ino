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
 *   - https://github.com/Pioneer-Valley-Open-Science/ethercard
 *     this one is updated for recent versions of avr-gcc
 *********************************************************************
 */

#include <EtherCard.h>
#include "time.h"

#define REQUEST_RATE 5000 // milliseconds

// ethernet interface mac address
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
// remote
const char ntpSite[] PROGMEM = "0.fr.pool.ntp.org";

byte Ethernet::buffer[700];
static long timer;
unsigned long ntpTime = 0;
unsigned long lastTime = 0;
unsigned long curTime = 0;

// called when the client request is complete
static void my_result_cb (byte status, word off, word len) {
  Serial.print("<<< reply ");
  Serial.print(millis() - timer);
  Serial.println(" ms");
  Serial.println((const char*) Ethernet::buffer + off);
}

/** get time from a ntp server
 *
 * original source: https://gist.github.com/futureshape/1328159
 *
 * @param ntpServer the ip of the server
 * @param ntpMyPort IP source port
 * @param timeZoneOffset offset to add to received time in sec
 */
unsigned long getNtpTime(uint8_t* ntpServer, uint16_t ntpMyPort, uint32_t timeZoneOffset) {
   unsigned long timeFromNTP;
   ether.ntpRequest(ntpServer, ntpMyPort);
   Serial.println("NTP request sent");
   while(true) {
      word length = ether.packetReceive();
      ether.packetLoop(length);
      if(length > 0 && ether.ntpProcessAnswer(&timeFromNTP,ntpMyPort)) {
         Serial.println("NTP reply received");
         return timeFromNTP - GETTIMEOFDAY_TO_NTP_OFFSET + timeZoneOffset;
      }
   }
   return 0;
}

void setup () {
  Serial.begin(57600);
  Serial.println("\n[getDHCPandDNSandNTP]");

  if (ether.begin(sizeof Ethernet::buffer, mymac, 10) == 0)
    Serial.println( "Failed to access Ethernet controller");

  if (!ether.dhcpSetup())
    Serial.println("DHCP failed");

  ether.printIp("My IP: ", ether.myip);
  // ether.printIp("Netmask: ", ether.mymask);
  ether.printIp("GW IP: ", ether.gwip);
  ether.printIp("DNS IP: ", ether.dnsip);

  if (!ether.dnsLookup(ntpSite))
    Serial.println("DNS failed");
  ether.printIp("Server: ", ether.hisip);

   ntpTime = getNtpTime(ether.hisip, 123, 3600);
   lastTime = millis();

  timer = - REQUEST_RATE; // start timing out right away
}


void loop () {
   char day[22];
   char clock[22];

   gmtime(ntpTime, day, clock);
   Serial.print( day );
   Serial.print( " " );
   Serial.println( clock );

   delay (30000);
   // update current date
   curTime = millis();
   ntpTime += (curTime - lastTime) / 1000;
   lastTime = curTime;

//    if (millis() > timer + REQUEST_RATE) {
//       timer = millis();
//       Serial.println("\n>>> REQ");
//       ether.browseUrl(PSTR("/foo/"), "bar", website, my_result_cb);
//    }
}
