
#ifndef _NETWORK_H_
#define _NETWORK_H_

bool set_mac(uint8_t *remIP);
long ntpDate(uint8_t* ntpsrvip);
void dhcpGet(void);
void printIP( uint8_t *buf );


#endif // _NETWORK_H_
