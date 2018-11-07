
#include "TCPSocket.h"
#pragma comment(lib, "Ws2_32.lib")

/*******************************************************************************
osInitTCP()
osFreeTCP()
******************************************************************************/
BOOL InitTCP()
{
    WSADATA wsaData;
    //---Инициализация WSA Windows, запрос спецификации 1.1
    if (WSAStartup(MAKEWORD(1, 1), &wsaData))        return FALSE;
    if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1)
    {
        WSACleanup();
        return FALSE;
    }
    return TRUE;
}

BOOL FreeTCP() { return (!WSACleanup()); }

/*******************************************************************************
*      Opens the tcp-client connection with remote  server on address given by tcpaddr:tcpport
*      Returns: socket handle if success or INVALID_SOCKET value, if error occurs
*/
SOCKET TCP_openccon(char*tcpaddr, unsigned tcpport)
{
    SOCKET s;
    struct sockaddr_in sin;
    int lasterror;
    fd_set rfds;
    timeval tout = { 2,0 };	/* 2 sec */

                              /*-----------------------------------------------------Open ARPA TCP Socket */
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) return INVALID_SOCKET;
    /*-------------------------------------------------------------------Binding*/
    sin.sin_family = AF_INET;	/* ARPA spec.address type*/
    sin.sin_port = 0;		/* Accept any free port number from tcp-pool*/
    sin.sin_addr.s_addr = 0;	/* Accept connection through any network interface*/
    if (0 != bind(s, (sockaddr*)&sin, sizeof(sockaddr_in)))
    {
    m1:     closesocket(s);
        return INVALID_SOCKET;
    }
    /*----------------------------------------------------------------Connecting*/
    sin.sin_family = AF_INET;
    sin.sin_port = htons(tcpport);
    sin.sin_addr.s_addr = inet_addr(tcpaddr);
    if (0 == connect(s, (sockaddr*)&sin, sizeof(sockaddr_in)))
    {
        FD_ZERO(&rfds); FD_SET(s, &rfds);
        select(1 + s, &rfds, NULL, NULL, &tout);
        if (FD_ISSET(s, &rfds))
            if (0 == recv(s, (char*)&lasterror, 1, MSG_PEEK))
                goto m1;
    }
    else  /*----------------Test a case if the server has refused the connection*/
        if ((lasterror = WSAGetLastError()) != WSAEISCONN)
            goto m1;
    return s;
}


/*******************************************************************************
*      Reads the tcp connection with remote  site
* Parameters:
*      s - tcp socket to read,
*      buf - buffer,
*      len - number of bytes to read,
*      tout - maximum reading time, seconds
* Returns:
*      number of bytes read on success, -1 on ERROR or if the connection were closed
*/
int TCP_readsoc(SOCKET s, unsigned char*buf, int len, unsigned tout)
{
    int i;
    fd_set rfds;
    timeval stval;

    if ((buf == 0) || (len <= 0))			return 0;
    stval.tv_sec = tout / 1000;
    stval.tv_usec = (tout % 1000) * 1000;
    /*-------------------------------------------------------------Reading*/
    FD_ZERO(&rfds); FD_SET((SOCKET)s, &rfds);
    select(1 + s, &rfds, NULL, NULL, &stval);
    if (FD_ISSET(s, &rfds))
    {
        i = recv(s, (char*)buf, len, /*MSG_NOSIGNAL*/0);
        if ((i == 0) ||			/* Connection closed by remote site*/
            (i == SOCKET_ERROR))		/* Socket error*/
        {
            //                int lasterror = WSAGetLastError();
            return -1;
        }
    }
    else i = 0;
    return i;
}

/*******************************************************************************
*      Writes the tcp connection with remote  site
* Parameters:
*      s - tcp socket to write,
*      buf - buffer,
*      len - number of bytes to read,
* Returns:
*      number of written bytes on success, -1 on ERROR
*/
int TCP_writesoc(SOCKET  s, unsigned char*buf, int len)
{
    int i;
    int BytesSent;

    if ((buf == 0) || (len <= 0))		return 0;
    BytesSent = send(s, (char*)buf, len, /*MSG_NOSIGNAL*/0);
    if (BytesSent == SOCKET_ERROR)
    {
        BytesSent = -1;
        //                int lasterror = WSAGetLastError();
    }
    return BytesSent;
}

/*******************************************************************************
*      Closes the tcp-client connection
*      s - tcp socket to close,
*/
int TCP_closesoc(SOCKET s)
{

    return closesocket(s); // client socket

}
