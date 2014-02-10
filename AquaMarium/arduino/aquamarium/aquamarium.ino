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
 * 2: ILS, 3: opto, 7: mid-tide
 * 4: motor fill, 5: motor empty
 *********************************************************************
 */

// to indicate that port b will not be used for pin change interrupts
#define NO_PORTB_PINCHANGES
// to indicate that port d will not be used for pin change interrupts
#define NO_PORTC_PINCHANGES
#define       DISABLE_PCINT_MULTI_SERVICE

#include <EtherCard.h>
#include <PinChangeInt.h>
#include "time.h"

#define REQUEST_RATE 5000 // milliseconds

// ethernet interface mac address
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
byte Ethernet::buffer[600];
// NTP data
const char ntpSite[] PROGMEM = "0.fr.pool.ntp.org";
unsigned long ntpTime = 0;
unsigned long lastTime = 0;
unsigned long curTime = 0;
// tide data
const char webSite[] PROGMEM = "diskstation";
boolean webOk = false;
unsigned long h1,h2;
float m1, m2;
// number of turns for mid-tide to high|low tide
#define MAX_TURN 100
// speed factor at which tide is going
#define SPEED_FACTOR 100
// numbers of rotations per 10 minutes for each tide hour
uint16_t rpm[] = {
   (1*(2*MAX_TURN/12))/6,
   (2*(2*MAX_TURN/12))/6,
   (3*(2*MAX_TURN/12))/6,
   (3*(2*MAX_TURN/12))/6,
   (2*(2*MAX_TURN/12))/6,
   (1*(2*MAX_TURN/12))/6
};

/** pump treatment variables */
#define PIN_ILS 2
#define PIN_OPTO 3
#define PIN_FILL 4
#define PIN_EMPTY 5
#define PIN_WATER 7
// number of turns for the pump
volatile int32_t nbTurns = 0;
#define FILL  1
#define EMPTY 0
#define STOP  2
// direction of the pump
volatile uint8_t direction = FILL;
// aproximate number of ms for one turn
#define TURN_DURATION 500
uint8_t previousWater;


/** get time from a ntp server
 *
 * original source: https://gist.github.com/futureshape/1328159
 *
 * @param ntpServer the name of the server
 * @param ntpMyPort IP source port
 * @param timeZoneOffset offset to add to received time in sec
 */
unsigned long getNtpTime(const char* ntpServer, uint16_t ntpMyPort, uint32_t timeZoneOffset) {
   unsigned long timeFromNTP;
   uint8_t ntpServerIp[4];

   Serial.println("ntp dns");
   if (!ether.dnsLookup(ntpServer)) {
      Serial.println("NTP DNS failed");
   }
   memcpy (ntpServerIp, ether.hisip, 4);
   ether.printIp("Server: ", ntpServerIp);

   ether.ntpRequest(ntpServerIp, ntpMyPort);
   Serial.println("NTP req");
   while(true) {
      word length = ether.packetReceive();
      ether.packetLoop(length);
      if(length > 0 && ether.ntpProcessAnswer(&timeFromNTP,ntpMyPort)) {
         Serial.println("NTP reply");
         return timeFromNTP - GETTIMEOFDAY_TO_NTP_OFFSET + timeZoneOffset;
      }
   }
   return ntpTime;
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
   Serial.print("incT:");
   Serial.println(nbTurns);
   interrupts();
} // incTurns

/** interrupt handler for water sensor
 */
void midTide (void) {
//    Serial.print("pin:"); Serial.println(PCintort::arduinoPin);
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
 * Warning: might be a bug when close to mid-tide
 *
 * @param forTurns number of turns
 */
void pump (int32_t forTurns) {
   int32_t targetTurns = nbTurns + forTurns;
   boolean goOn = true;
   uint16_t val = 0;
   uint8_t curWater = previousWater;

   Serial.print("pump:");
   Serial.println (forTurns);

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

/** goes to mid tide by pumping water in or out
 */
void gotoMidTide (void) {
   uint8_t cW = digitalRead (PIN_WATER);
   uint8_t nT = (cW > 0)? -1 : 1;
   Serial.println("gotomid");

   while (cW == digitalRead(PIN_WATER)) {
      pump(nT);
   }
} // gotoMidTide

// called when the web client request is complete
static void web_cb (byte status, word off, word len) {
   char *p,*i;

//    Serial.println((const char*) Ethernet::buffer + off);
   // 1391952287;6.70;1391974907;3.85
   p = strtok_r ((char*)Ethernet::buffer + off, ";", &i); // header
   p = strtok_r (NULL, ";", &i); // h1
//    Serial.print("h1 "); Serial.println(p);
   h1 = atol(p);
   Serial.print (h1);
   p = strtok_r (NULL, ";", &i); // m1
   m1 = atof(p);
   Serial.print(":"); Serial.println(m1);
   p = strtok_r (NULL, ";", &i); // h2
   h2 = atol(p);
//    Serial.print("h2 "); Serial.println(p);
   Serial.print (h2);
   p = strtok_r (NULL, ";", &i); // m2
   m2 = atof(p);
   Serial.print(":"); Serial.println(m2);

   webOk = true;
}

/** get the current tide from webserver
 */
void getTide (void) {
   Serial.println("Web DNS");
   if (!ether.dnsLookup(webSite)) {
      Serial.println("WEB DNS failed");
   }
   ether.printIp("SRV: ", ether.hisip);
   Serial.println("web GET");
   ether.browseUrl(PSTR("/tide/"), "geth.php", webSite, web_cb);
   while (webOk == false) {
      ether.packetLoop(ether.packetReceive());
   }
} // getTide

/************************ main setup *********************************/
void setup () {
   Serial.begin(57600);
   Serial.println("\n== AquaMarium ==");

   // CS for the used ethershield is pin 10
   if (ether.begin(sizeof Ethernet::buffer, mymac, 10) == 0) {
      Serial.println( "Failed Ethernet");
   }

   Serial.println("dhcp req");
   if (!ether.dhcpSetup()) {
      Serial.println("DHCP failed");
   }

   ether.printIp("My IP: ", ether.myip);
   // ether.printIp("Netmask: ", ether.mymask);
   ether.printIp("GW IP: ", ether.gwip);
   ether.printIp("DNS IP: ", ether.dnsip);

   /** pump initialisation */
   pinMode (PIN_FILL,  OUTPUT);
   pinMode (PIN_EMPTY, OUTPUT);
//    pinMode (PIN_ILS, INPUT);
   pinMode (PIN_WATER, INPUT);
   // INT1 --> arduino pin 3
   // INT0 --> arduino pin 2
//    attachInterrupt ( 0, incTurns, RISING);
   PCintPort::attachInterrupt(PIN_WATER, &midTide, CHANGE);
   PCintPort::attachInterrupt(PIN_ILS, &incTurns, RISING);
   interrupts();
//    previousWater = digitalRead (PIN_WATER);
} // setup()

/************************ main loop *********************************/
void loop () {
   char day[22];
   char clock[22];
   int32_t n;
   int8_t  sens = 1, i=0, init=1;

   while (1) {
   //    ntpTime = getNtpTime(ntpSite, 123, 3600);
      ntpTime = getNtpTime(ntpSite, 123, 0);
   //    ntpTime = 1391961398; // DEBUG
      lastTime = millis();

      getTide();

      // display current date
      gmtime(ntpTime, day, clock);
      Serial.print( day );
      Serial.print( " " );
      Serial.println( clock );

      if (init == 1) {
         // calibration
         gotoMidTide();
         // goto current tide
         Serial.println("gotocurr");
         if (m1 > m2) {
            // jusant
            n = map (ntpTime, h1, h2, MAX_TURN, -MAX_TURN);
            sens = -1;
         } else {
            // flot
            n = map (ntpTime, h1, h2, -MAX_TURN, MAX_TURN);
            sens = 1;
         }
         pump (n);
         init = 0;
      }

      // do the tide
      while (ntpTime <= h2) {
         i = map (ntpTime, h1, h2, 0, 6); // get the current rpm index
         pump( sens * rpm[i] );
         delay(600000/SPEED_FACTOR); // sleep 10 min
         // update current date
         curTime = millis();
         ntpTime += ((curTime - lastTime) / 1000) * SPEED_FACTOR;
         lastTime = curTime;
      }
   } // while 1
} // loop()
