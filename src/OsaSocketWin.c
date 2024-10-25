/****************************************************************************/
/*                                                                          */
/*   Filename   : OsaSocketWin.c                                            */
/*                                                                          */
/*   Description: OS abstraction layer                                      */
/*                OS abstraction for Windows Socket                         */
/*                                                                          */
/*   Copyright  :                                                           */
/*                                                                          */
/****************************************************************************/
#define OSA_INTERNAL

#include <stdio.h>
#include <winsock2.h>

#include "OsaError.h"
#include "OsaSocket.h"

/* Request Winsock version 2.2
 *
 * The highest version of Windows Sockets specification that the caller
 * can use. The high-order byte specifies the minor version number;
 * the low-order byte specifies the major version number.
 */
#define SOCKLIB_VERSION2     MAKEWORD(2,2)

#pragma warning(disable : 4996) // ***remove it later***

/* Covert OsaSocketAddr to Windows sockaddr_in struct.*/
static void OsaToWinAddr(const OsaSocketAddr* osaAddr, struct sockaddr_in* winAddr);

/* Covert Windows sockaddr_in to OsaSocketAddr struct.*/
static void WinToOsaAddr(const struct sockaddr_in* winAddr,  OsaSocketAddr* osaAddr);

/*
 *      Socket Creation API.
 */
OsaErr
OsaSocketCreate(OsaSocketProtocol  protocol,
                OsaSocket*        sock)
{
    OsaErr status = OSA_EOK;
    int sockType  = 0;
    int sockProto = 0;

    OSA_ASSERT(sock != NULL);

    switch(protocol)
    {
    case OSA_SOCKETTYPE_TCP:

        sockType  = SOCK_STREAM;
        sockProto = IPPROTO_TCP;
        break;

    case OSA_SOCKETTYPE_UDP:

        sockType  = SOCK_DGRAM;
        sockProto = IPPROTO_UDP;
        break;

    default:

        status = OSA_EPARAM;
        break;
    }

    if (status == OSA_EOK)
    {
        *sock = WSASocket(AF_INET, sockType, sockProto, NULL, 0u, WSA_FLAG_NO_HANDLE_INHERIT | WSA_FLAG_OVERLAPPED);

        status = (*sock == INVALID_SOCKET) ? OSA_ENOK : OSA_EOK;

        OSA_HANDLE_WSAGETLASTERR(OSA_GET_ERR);
    }

    return status;
}

/*
 *      Bind a name to a socket.
 */
OsaErr
OsaSocketBind(OsaSocket            sock,
              const OsaSocketAddr* name)
{
    INT32              status;
    struct sockaddr_in addr;
    OsaErr             osaErr = OSA_EOK;

    OSA_ASSERT(name != NULL);

    OsaToWinAddr(name, &addr);

    status = bind(sock, (SOCKADDR *)&addr, sizeof(addr));                      

    if(status == SOCKET_ERROR)
    {
        status = WSAGetLastError();

        if(status == WSAEADDRNOTAVAIL)
        {
            osaErr = OSA_EPARAM;
        }
        else
        {
            OSA_HANDLE_WSAGETLASTERR(status);
            osaErr = OSA_ENOK;
        }
    }

    return osaErr;
}

/*
 *      Enables connections to a socket.
 */
OsaErr
OsaSocketListen(OsaSocket sock,
                INT32     backlog)
{
    INT32 status = listen(sock, backlog);

    OSA_HANDLE_WSAGETLASTERR(OSA_GET_ERR);

    return (status == SOCKET_ERROR) ? OSA_ENOK : OSA_EOK;
}

/*
 *      Accept a connection from a socket.
 */
OsaErr
OsaSocketAccept(OsaSocket      sock,
                OsaSocketAddr* newName,
                OsaSocket*     newSock)
{
    INT32 newNameLength = 0;

    struct sockaddr_in addr;

    OSA_ASSERT(newName != NULL);
    OSA_ASSERT(newSock != NULL);

    newNameLength = sizeof(addr);

    *newSock = accept(sock, (SOCKADDR *) &addr, &newNameLength);             

    if (*newSock != INVALID_SOCKET)
    {
        WinToOsaAddr(&addr, newName);
    }

    OSA_HANDLE_WSAGETLASTERR(OSA_GET_ERR);
    
    return (*newSock == INVALID_SOCKET) ? OSA_ENOK : OSA_EOK;
}

/*
 *      Initiate a connection to a socket
 */
OsaErr
OsaSocketConnect(OsaSocket            client,
                 const OsaSocketAddr* server)
{
    INT32 status;

    struct sockaddr_in winAddr;

    OSA_ASSERT(server != NULL);

    OsaToWinAddr(server, &winAddr);

    status = connect(client, (SOCKADDR *) &winAddr, sizeof(winAddr));      

    OSA_HANDLE_WSAGETLASTERR(OSA_GET_ERR);

    return (status == SOCKET_ERROR) ? OSA_ENOK : OSA_EOK;
}

/*
 *      Initiate a connection to a socket within timeout
 */
OsaErr
OsaSocketConnectWithTimeout(OsaSocket            client,
                            const OsaSocketAddr* server,
                            const OsaTimeVal*    timeOut)
{
    BOOL    connected = FALSE;
    INT32   status;
    UINT32  non_block_opt;

    struct sockaddr_in peer;

    OSA_ASSERT(server  != NULL);
    OSA_ASSERT(timeOut != NULL);
    
    OsaToWinAddr(server, &peer);

    /* Make the call to connect non blocking */

    non_block_opt = TRUE;
    OsaSocketIoctl(client, OSA_FIONBIO, (UINT32*)&non_block_opt);
    status = connect(client, (SOCKADDR *)&peer, sizeof(peer));
    connected = (status != SOCKET_ERROR);

    if ( ! connected)
    {
        status = WSAGetLastError();

        if ((status == WSAEWOULDBLOCK) || (status == WSAEINPROGRESS) || (status == WSAEALREADY))
        {
            /*  Wait for connection until timeout or success is reported in writefds */

            /* Attach a writeable filedescriptor to the socket */

            fd_set Write;
                                                                              
            FD_ZERO(&Write);

            #pragma warning( disable : 4127 )

            FD_SET(client, &Write);

            #pragma warning( default : 4127 )
                                                                              
            status = select(0, 0, &Write, 0, timeOut);

            /* Test on timeout */

            connected = (status > 0);

            if (connected)
            {
                connected = FD_ISSET(client, &Write);                        

                /* Restore blocking */
                non_block_opt = FALSE;
                OsaSocketIoctl(client, OSA_FIONBIO, (UINT32*)&non_block_opt);
            }
        }
    }

    OSA_HANDLE_WSAGETLASTERR(OSA_GET_ERR);

    return (connected) ? OSA_EOK : OSA_ENOK;
}

/*
 *      Send a message to a socket
 */
OsaErr
OsaSocketSendTo(OsaSocket            sock,
                const CHAR*          buf,
                INT32                bufLen,
                OsaSocketMsgFlags    flags,
                const OsaSocketAddr* toAddr,
                INT32*               bytesSent)
{
    INT32   status;

    struct sockaddr_in addr;

    OSA_ASSERT(buf       != NULL);
    OSA_ASSERT(toAddr    != NULL);
    OSA_ASSERT(bytesSent != NULL);

    OsaToWinAddr(toAddr, &addr);

    status = sendto(sock, buf, bufLen, flags, (SOCKADDR *) &addr, sizeof(addr));             

    OSA_HANDLE_WSAGETLASTERR(OSA_GET_ERR);

    *bytesSent = (status != SOCKET_ERROR) ? status : 0;
    return (status == SOCKET_ERROR) ? OSA_ENOK : OSA_EOK;
}

/*
 *      Send a message to a socket
 */
OsaErr
OsaSocketSend(OsaSocket         sock,
              const CHAR*       buf,
              INT32             bufLen,
              OsaSocketMsgFlags flags,
              INT32*            bytesSent)
{
    INT32   status;

    OSA_ASSERT(buf       != NULL);
    OSA_ASSERT(bytesSent != NULL);

    status = send(sock, buf, bufLen, flags);



    *bytesSent = (status != SOCKET_ERROR) ? status : 0;
    return (status == SOCKET_ERROR) ? OSA_ENOK : OSA_EOK;
}

/*
 *      receive a message from a socket
 */
OsaErr
OsaSocketRecvFrom(OsaSocket         sock,
                  CHAR*             buf,
                  INT32             bufLen,
                  OsaSocketMsgFlags flags,
                  OsaSocketAddr*    fromAddr,
                  INT32*            rcvBytes)
{
    OsaErr status = OSA_ENOK;

    INT32  fromAddrLen;

    struct sockaddr_in addr;

    OSA_ASSERT(buf      != NULL);
    OSA_ASSERT(fromAddr != NULL);
    OSA_ASSERT(rcvBytes != NULL);

    fromAddrLen = sizeof(addr);

    *rcvBytes = recvfrom(sock, buf, bufLen, flags, (SOCKADDR *)&addr, &fromAddrLen);          
    
    if (*rcvBytes == SOCKET_ERROR)
    {
        /* recvfrom failed. */
        status = OSA_ENOK;
        
        OSA_HANDLE_WSAGETLASTERR(OSA_GET_ERR);
    }
    else if (*rcvBytes == 0)
    {
        /* The connection has been gracefully closed. */
        status = OSA_ENOK;
    }
    else
    {
        /* Bytes received. */
        status = OSA_EOK;
    }

    if (status == OSA_EOK)
    {
        WinToOsaAddr(&addr, fromAddr);
    }
    else
    {
        *rcvBytes = 0;
    }

    return status;
}

/*
 *      receive a message from a socket 
 */
OsaErr
OsaSocketRecv(OsaSocket         sock,
              CHAR*             buf,
              INT32             bufLen,
              OsaSocketMsgFlags flags,
              INT32*            rcvBytes)
{
    OsaErr status = OSA_ENOK;

    OSA_ASSERT(buf      != NULL);
    OSA_ASSERT(rcvBytes != NULL);

    *rcvBytes = recv(sock, buf, bufLen, flags);

    if (*rcvBytes == SOCKET_ERROR)
    {
        if (WSAGetLastError() == WSAETIMEDOUT)
        {
            status = OSA_ETIME;
        }
        else
        {
            status = OSA_ENOK;
            *rcvBytes = 0;
        }

        OSA_HANDLE_WSAGETLASTERR(OSA_GET_ERR);
    }
    else if (*rcvBytes == 0)
    {
        /* The connection has been gracefully closed. */
        status = OSA_ENOK;
    }
    else
    {
        /* Bytes received. */
        status = OSA_EOK;
    }

    return status;
}

/*
 *      function closes an existing socket.
 */
OsaErr
OsaSocketClose(OsaSocket sock)
{
    INT32 status = closesocket(sock);

    OSA_HANDLE_WSAGETLASTERR(OSA_GET_ERR);

    return (status == SOCKET_ERROR) ? OSA_ENOK : OSA_EOK;
}

/*
 *      set socket options
 */
OsaErr
OsaSocketSetOpt(OsaSocket          sock,
                OsaSocketProtoLvl  level,
                OsaSocketOptName   optName,
                const CHAR*        optValue,
                INT32              optValueLen)
{
    INT32 status;
    int   lvl     = 0;
    OsaErr osaErr = OSA_EOK;

    OSA_ASSERT(optValue != NULL);
    
    switch (level)
    {
    case OSA_SOCKETOPTLVL_SOL:
        lvl = SOL_SOCKET;
        break;

    case OSA_SOCKETOPTLVL_TCP:
        lvl = IPPROTO_TCP;
        break;

    default:
        osaErr = OSA_EPARAM;
        break;
    }

    if (osaErr == OSA_EOK)
    {
        if (((OSA_SOCKETOPTLVL_SOL == level) && (SO_RCVTIMEO == optName)) ||
            ((OSA_SOCKETOPTLVL_SOL == level) && (SO_SNDTIMEO == optName)))
        {
            //Some bug in the socket lib of Windows;
            //a value between 0 and 0x7FFFFFFF will be handled
            //as timeout where 0 is infinite
            //a value between 0x80000000 and 0xFFFFFFFF will return immediately,
            //because it is interpreted as a negative value
            //so every value greater than 0x7FFFFFFF will be handled as infinite
            INT32 to = *((INT32*)optValue);
            if (0 > to)
            {
                to = 0;
            }
            status = setsockopt(sock, lvl, optName, (char*)&to, optValueLen);
        }
        else
        {
            status = setsockopt(sock, lvl, optName, optValue, optValueLen);
        }

        OSA_HANDLE_WSAGETLASTERR(OSA_GET_ERR);

        osaErr = (status == SOCKET_ERROR) ? OSA_ENOK : OSA_EOK;
    }

    return osaErr;
}

/*
 *      get socket options
 */
OsaErr
OsaSocketGetOpt(OsaSocket          sock,
                OsaSocketProtoLvl  level,
                OsaSocketOptName   optName,
                CHAR*              optValue,
                INT32*             optValueLen)
{
    INT32 status;
    int   lvl     = 0;
    OsaErr osaErr = OSA_EOK;

    OSA_ASSERT(optValue     != NULL);
    OSA_ASSERT(optValueLen  != NULL);

    switch (level)
    {
    case OSA_SOCKETOPTLVL_SOL:
        lvl = SOL_SOCKET;
        break;

    case OSA_SOCKETOPTLVL_TCP:
        lvl = IPPROTO_TCP;
        break;

    default:
        osaErr = OSA_EPARAM;
        break;
    }

    if (osaErr == OSA_EOK)
    {
        status = getsockopt(sock, lvl, optName, optValue, optValueLen);

        OSA_HANDLE_WSAGETLASTERR(OSA_GET_ERR);

        osaErr = (status == SOCKET_ERROR) ? OSA_ENOK : OSA_EOK;
    }

    return osaErr;
}

/*
 *      The OsaSocketGetName function retrieves the local name for a socket.
 */
OsaErr
OsaSocketGetName(OsaSocket      sock,
                 OsaSocketAddr* name)
{
    INT32 status;
    INT32 nameLen;

    struct sockaddr_in addr;

    OSA_ASSERT(name != NULL);

    nameLen = sizeof(addr);

    status = getsockname(sock, (SOCKADDR*)&addr, &nameLen);                  

    if (status != SOCKET_ERROR)
    {
        WinToOsaAddr(&addr, name);
    }

    OSA_HANDLE_WSAGETLASTERR(OSA_GET_ERR);

    return (status == SOCKET_ERROR) ? OSA_ENOK : OSA_EOK;
}

/*
 *      The OsaSocketGetPeerName function retrieves the address of the peer to which
 *      a socket is connected.
 */
OsaErr
OsaSocketGetPeerName(OsaSocket      sock,
                     OsaSocketAddr* name)
{
    INT32 status;
    INT32 nameLen;

    struct sockaddr_in addr;

    OSA_ASSERT(name != NULL);
    
    nameLen = sizeof(addr);

    status = getpeername(sock, (SOCKADDR*)&addr, &nameLen);           

    if (status != SOCKET_ERROR)
    {
        WinToOsaAddr(&addr, name);
    }

    OSA_HANDLE_WSAGETLASTERR(OSA_GET_ERR);

    return (status == SOCKET_ERROR) ? OSA_ENOK : OSA_EOK;
}

/*
 *      The shutdown function disables sends and/or receives on a socket.
 */
OsaErr
OsaSocketShutdown(OsaSocket             sock,
                  OsaSocketShutdownType how)
{
    INT32 status = shutdown(sock, how);

    OSA_HANDLE_WSAGETLASTERR(OSA_GET_ERR);

    return (status == SOCKET_ERROR) ? OSA_ENOK : OSA_EOK;
}

/*
 *      The OsaSocketIoctl function controls the I/O mode of a socket.
 */
OsaErr
OsaSocketIoctl(OsaSocket   sock,
               OsaSocketIo cmd,
               UINT32*     args)
{
    OsaErr status = OSA_EOK;

    OSA_ASSERT(args != NULL);

    if ((cmd != OSA_FIONBIO) && (cmd != OSA_FIONREAD))
    {
        status = OSA_EPARAM;
    }

    if (status == OSA_EOK)
    {
        long ioCmd = (cmd == OSA_FIONBIO) ? FIONBIO :
                                            FIONREAD;
        int err = ioctlsocket(sock, ioCmd, (u_long*)args);          

        status = (err == SOCKET_ERROR) ? OSA_ENOK : OSA_EOK;
    }
    
    OSA_HANDLE_WSAGETLASTERR(OSA_GET_ERR);

    return status;
}

/*
 *      Initiates use of the socket functions by a process.
 */
OsaErr
OsaSocketStartup(void)
{
    WSADATA wsaData = {0};

    INT32 Result = WSAStartup(SOCKLIB_VERSION2, &wsaData);         

    return (Result == 0) ? OSA_EOK : OSA_ENOK;
}

/*
 *      Terminates use of the socket functions by a process.
 */
OsaErr
OsaSocketCleanup(void)
{
    INT32 Result = WSACleanup();

    return (Result == 0) ? OSA_EOK : OSA_ENOK;
}

#pragma warning( disable : 4702 )          /* warning C4702: unreachable code */

/*
 *      Function to fill the OsaSocketAddr structure.
 */
OsaErr
OsaSocketSetAddress(OsaSocketAddr* addr,
                    const CHAR*    ipAddr,
                    UINT16         port)
{
    OsaErr      status = OSA_EOK;
    OsaInAddr   inAddr;

    OSA_ASSERT(addr   != NULL);
    
    memset(addr, 0, sizeof(*addr));

    if (ipAddr == NULL)
    {
        addr->address = INADDR_ANY;
    }
    else
    {
        if  (strlen(ipAddr) == 0)
        {
            addr->address = INADDR_ANY;
        }
        else
        {
            status = OsaInetAddr(ipAddr, &inAddr);
            addr->address = inAddr;
        }
    }

    addr->family = AF_INET;
    addr->port   = port;

    return status;
}

#pragma warning( default : 4702 )

/*
 *      Converts a string containing an IPv4 dotted-decimal address
 *      into a proper address for the OsaInAddr structure.
 */
OsaErr
OsaInetAddr(const CHAR* cp,
            OsaInAddr*  inetAddr)
{
    OSA_ASSERT(cp       != NULL);
    OSA_ASSERT(inetAddr != NULL);

    *inetAddr = ntohl(inet_addr(cp));

    return OSA_EOK;
}

/*
 *      Function converts an (Ipv4) Internet network address into an ASCII string
 *      in Internet standard dotted-decimal format.
 */
OsaErr
OsaInetNtoa(OsaInAddr in,
            CHAR**    out)
{
    struct in_addr  inAddr;

    OSA_ASSERT(out != NULL);

    inAddr.S_un.S_addr = htonl(in);

    *out = inet_ntoa(inAddr);

    return (*out == NULL) ? OSA_ENOK : OSA_EOK;
}

/*
 *      Function converts an OsaSocketAddr struct to a Windows struct sockaddr_in
 */
void
OsaToWinAddr(const OsaSocketAddr*      osaAddr,
                   struct sockaddr_in* winAddr)
{
    OSA_ASSERT(osaAddr != NULL);
    OSA_ASSERT(winAddr != NULL);

    memset(winAddr, 0, sizeof(*winAddr));
    winAddr->sin_addr.S_un.S_addr = htonl(osaAddr->address);
    winAddr->sin_family           = osaAddr->family;

    /* TODO: use htons later in the blow line of code */
    winAddr->sin_port             = (osaAddr->port);
}

/*
 *      Function converts a Windows struct sockaddr_in to an OsaSocketAddr struct 
 */
void
WinToOsaAddr(const struct sockaddr_in* winAddr,
                   OsaSocketAddr*      osaAddr)
{
    OSA_ASSERT(winAddr != NULL);
    OSA_ASSERT(osaAddr != NULL);

    osaAddr->address = ntohl(winAddr->sin_addr.S_un.S_addr);
    osaAddr->family  = winAddr->sin_family;
    osaAddr->port    = ntohs(winAddr->sin_port);
}

