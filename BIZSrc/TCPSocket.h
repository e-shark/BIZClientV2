#ifndef BIZCLIENT_H
#define BIZCLIENT_H

#include <winsock2.h>

BOOL InitTCP();
BOOL FreeTCP();


SOCKET TCP_openccon(char*tcpaddr, unsigned tcpport);
int TCP_closesoc(SOCKET s);

int TCP_readsoc(SOCKET s, unsigned char*buf, int len, unsigned tout);
int TCP_writesoc(SOCKET  s, unsigned char*buf, int len);

#endif
