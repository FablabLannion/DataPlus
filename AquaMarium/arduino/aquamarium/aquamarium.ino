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
 *   - EtherCad https://github.com/Pioneer-Valley-Open-Science/ethercard
 *     this one is updated for recent versions of avr-gcc
 *   - PinChangeInt http://code.google.com/p/arduino-pinchangeint/
 *********************************************************************
 * Pinout :
 * 2: ILS, 3: opto, 6: mid-tide
 * 4: motor fill, 5: motor empty
 *********************************************************************
 */

// // to indicate that port b will not be used for pin change interrupts
// #define NO_PORTB_PINCHANGES
// // to indicate that port d will not be used for pin change interrupts
// #define NO_PORTC_PINCHANGES
// #define       DISABLE_PCINT_MULTI_SERVICE

#include <EtherCard.h>
// #include <PinChangeInt.h>
#include "time.h"

#define REQUEST_RATE 5000 // milliseconds

// ethernet interface mac address
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
byte Ethernet::buffer[700];
// NTP data
const char ntpSite[] PROGMEM = "0.fr.pool.ntp.org";
uint8_t    ntpServerIp[4];
unsigned long ntpTime = 0;
unsigned long lastTime = 0;
unsigned long curTime = 0;

// called when the client request is complete
// static void my_result_cb (byte status, word off, word len) {
//    Serial.print("<<< reply ");
//    Serial.print(millis() - timer);
//    Serial.println(" ms");
//    Serial.println((const char*) Ethernet::buffer + off);
// }

/** pump treatment variables */
#define PIN_ILS 2
#define PIN_OPTO 3
#define PIN_FILL 4
#define PIN_EMPTY 5
#define PIN_WATER 6
// number of turns for the pump
volatile int32_t nbTurns = 0;
#define FILL  1
#define EMPTY 0
#define STOP  2
// direction of the pump
volatile uint8_t direction = FILL;
// aproximate number of ms for one turn
#define TURN_DURATION 1000


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
} // getNtpTime

/** interrupt handler for reed sensor
 */
void incTurns (void) {
//    Serial.print("pin:"); Serial.println(PCintPort::arduinoPin);
   noInterrupts();
   if (direction == FILL) {
      nbTurns++;
   } else {
      nbTurns--;
   }
   Serial.println(nbTurns);
   interrupts();
} // incTurns

/** interrupt handler for water sensor
 */
void midTide (void) {
//    Serial.print("pin:"); Serial.println(PCintPort::arduinoPin);
   Serial.print ("mi-maree: ");
   Serial.println (nbTurns);
   nbTurns = 0;
} // midTide

/** pump for a certain amount of turns
 *
 * number of turns can be :
 *  >0 : fill the tank
 *  <0 : empty the tank
 *  =0 : stop the pump
 *
 * @param forTurns number of turns
 */
void pump (int32_t forTurns) {
   int32_t targetTurns = nbTurns + forTurns;
   boolean goOn = true;
   uint16_t val = 0;

   if (forTurns > 0) {
      direction = FILL;
      digitalWrite ( PIN_FILL, HIGH);
      digitalWrite ( PIN_EMPTY, LOW);
   } else if (forTurns < 0) {
      direction = EMPTY;
      digitalWrite ( PIN_FILL, LOW);
      digitalWrite ( PIN_EMPTY, HIGH);
   } else {
      direction = STOP;
      goOn = false;
      digitalWrite ( PIN_EMPTY, LOW);
      digitalWrite (PIN_FILL, LOW);
   }
   // wait for nbTurns to reach targetTurns
   while (goOn == true) {
//       Serial.print ("turns ");
//       Serial.print (nbTurns); Serial.print (" / ");
//       Serial.println (targetTurns);
      delay(TURN_DURATION);
      if (direction == FILL) {
         goOn = (nbTurns < targetTurns);
      } else {
         goOn = (nbTurns > targetTurns);
      }
   }
   direction = STOP;
} // pump

void setup () {
   Serial.begin(57600);
   Serial.println("\n== AquaMarium ==");

   // CS for the used ethershield is pin 10
   if (ether.begin(sizeof Ethernet::buffer, mymac, 10) == 0) {
      Serial.println( "Failed to access Ethernet controller");
   }

   if (!ether.dhcpSetup()) {
      Serial.println("DHCP failed");
   }

   ether.printIp("My IP: ", ether.myip);
   // ether.printIp("Netmask: ", ether.mymask);
   ether.printIp("GW IP: ", ether.gwip);
   ether.printIp("DNS IP: ", ether.dnsip);

//    if (!ether.dnsLookup(ntpSite)) {
//       Serial.println("NTP DNS failed");
//    }
//    memcpy (ntpServerIp, ether.hisip, 4);
//    ether.printIp("Server: ", ntpServerIp);
//
//    ntpTime = getNtpTime(ntpServerIp, 123, 3600);
   ntpTime = 1391865943; // DEBUG
   lastTime = millis();

   /** pump initialisation */
   pinMode (PIN_FILL,  OUTPUT);
   pinMode (PIN_EMPTY, OUTPUT);
//    pinMode (PIN_ILS, INPUT);
//    pinMode (PIN_WATER, INPUT);
   // INT1 --> arduino pin 3
   // INT0 --> arduino pin 2
   attachInterrupt ( 0, incTurns, RISING);
//    PCintPort::attachInterrupt(PIN_WATER, &midTide, CHANGE);
//    PCintPort::attachInterrupt(PIN_ILS, &incTurns, RISING);
   interrupts();
}


void loop () {
   char day[22];
   char clock[22];

   gmtime(ntpTime, day, clock);
   Serial.print( day );
   Serial.print( " " );
   Serial.println( clock );

   pump (10);
   pump (-10);

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
