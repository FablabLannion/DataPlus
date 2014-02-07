
#include "network.h"

// Output a ip address from buffer
void printIP( uint8_t *buf ) {
   int i;
  for(i = 0; i < 4; i++ ) {
    Serial.print( buf[i], DEC );
    if( i<3 )
      Serial.print( "." );
  }
}


bool set_mac(uint8_t *remIP){
  uint32_t endtime,dat_p;
  uint16_t timeout=2000; //timeout ping after 500 milli seconds
  endtime=millis();
  Serial.print("Senidng ARP Request to ");printIP(remIP);Serial.println();
  es.ES_client_set_gwip(remIP);
    while(1){
      if(millis()-endtime>=timeout){Serial.print("Timeout Waitin for Ping Reply ");Serial.print(endtime);Serial.println();return false;}//timeout ping
       int plen=es.ES_enc28j60PacketReceive(BUFFER_SIZE,buf);
       dat_p=es.ES_packetloop_icmp_tcp(buf,plen);
       if(dat_p==0) {
          // we are idle here
          if (es.ES_client_waiting_gw() ){
          continue;
          }

        }
        if (!es.ES_client_waiting_gw() ){Serial.println("Got ARP Reply");return true;}

}

}


long ntpDate(uint8_t* ntpsrvip)
{
   uint16_t dat_p;

   while (timeLong == 0) {
      set_mac(ntpsrvip);
      Serial.println( F("Send NTP request " ));
      es.ES_client_ntp_request(buf, ntpsrvip, clientPort);
      Serial.print( F("clientPort: "));
      Serial.println(clientPort, DEC );
      // handle ping and wait for a tcp packet
      dat_p=es.ES_packetloop_icmp_tcp(buf,es.ES_enc28j60PacketReceive(BUFFER_SIZE, buf));
      // Has unprocessed packet response
      if (dat_p > 0)
      {
         timeLong = 0L;
         if (es.ES_client_ntp_process_answer(buf, &timeLong,clientPort)) {
            Serial.print( F( "Time:" ));
            Serial.println(timeLong); // secs since year 1900
         }
      }
   }
   return timeLong;
} // ntpDate

void dhcpGet (void) {
     Serial.println("Requesting IP Addresse");
  // Get IP Address details
  if( es.allocateIPAddress(buf, BUFFER_SIZE, mymac, 80, myip, mynetmask, gwip, dhcpsvrip, dnsip ) > 0 ) {
    // Display the results:
    Serial.print( "My IP: " );
    printIP( myip );
    Serial.println();

    Serial.print( "Netmask: " );
    printIP( mynetmask );
    Serial.println();

    Serial.print( "DNS IP: " );
    printIP( dnsip );
    Serial.println();

    Serial.print( "GW IP: " );
    printIP( gwip );
    Serial.println();
  } else {
    Serial.println("Failed to get IP address");
  }

} // dhcpGet
