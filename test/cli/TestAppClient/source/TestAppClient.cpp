// TestAppClient.cpp : Defines the entry point for the console application.
//



#define WIN32_LEAN_AND_MEAN
#define forever                  for(;;)


#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "Osa.h"

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512

#define DEFAULT_PORT 26395 // ----> 7015
// #define DEFAULT_PORT "26395"

const CHAR* remoteIpAddress = " ";
const UINT32   SenderTimeOutInMs = 2000;
const UINT32   SenderFrequencyInMs = 500;
const UINT32   ReceiveTimeOutInMs = 2000;
const UINT32   ReceiveFrequencyInMs = 500;

WSADATA wsaData;
OsaSocket ClientSocket = INVALID_SOCKET;
//OsaErr osaErr = OSA_EOK;
OsaSocketAddr remoteAddress;
UINT16 remotePort = DEFAULT_PORT;
INT32 BytesSent = 0;
INT32 BytesReceived = 0;
UINT8 Bytes = 0;
char rawstring[512];

const char *sendbuf = "this is a test";
char recvbuf[DEFAULT_BUFLEN];
int recvbuflen = DEFAULT_BUFLEN;

/* Thread related definitions */
#define RECEIVE_TASK_STACK_SIZE        (12*1024)      
#define RECEIVE_TASK_PRIORITY           60   
#define RECEIVE_TASK_PRIORITY2          40
#define MIN_TRANSLATE_PRIORITY         196 
#define SEND_TASK_PRIORITY             (12*1024)

static OsaThreadId  sck2mbxThreadId;
void Receive_msg_socket2mbox_task(void);

static OsaThreadId mbx2sckThreadId;
void Send_msg_mbox2socket_task(void);


/* COMMAND LINE NTERFACE DEFINATIONS*/
static OsaThreadId commandinterface;
void CommandLineIntTask(void);
#define MAX_CMD_STR_LEN                   21
static CHAR cmdString[MAX_CMD_STR_LEN] = { NULL };
OsaSyncId mutex;



int __cdecl main(int argc, char **argv)
{

    // OSA Initilization
    OsaInitLib initLib = { NULL,
                          NULL,
                          TRUE,
                          TRUE };

    OsaErr osaErr = OsaInit(&initLib);

    if (OSA_EOK == osaErr)
    {
        (void)OsaFilePrintF("OSA Inialization Sucessful!\n");
    }
    else
    {
        (void)OsaFilePrintF("OSA Inialization Failed! Error: %d", osaErr);
    }

    // Validate the parameters
    if (argc != 2) {
        (void)OsaFilePrintF("usage: %s server-name\n", argv[0]);
        return 1;
    }

    remoteIpAddress = argv[1];
    (void)OsaFilePrintF("DEBUG: argv[1]: %s, remoteIpAddress: %s\n", argv[1], remoteIpAddress);

    /* Create a TCP Socket */
    if (OSA_EOK == osaErr)
    {
        osaErr = OsaSocketCreate(OSA_SOCKETTYPE_TCP, &ClientSocket);
    }
    if (osaErr != OSA_EOK)
    {
        // TODO: Create a generic log infrastructure later, CAT compliant (CO_Log_XYZ)
        (void)OsaFilePrintF("Error occurred while creating socket, osaErr: %d", osaErr);
    }
    else
    {
        (void)OsaFilePrintF("DEBUG: socket created successfully, osaErr: %d", osaErr);
        osaErr = OsaSocketSetAddress(&remoteAddress, remoteIpAddress, remotePort);
    }

    if (osaErr != OSA_EOK)
    {
        // TODO: Create a generic log infrastructure later, CAT compliant (CO_Log_XYZ)
        (void)OsaFilePrintF("Error occurred while OsaSocketSetAddress, osaErr: %d\n", osaErr);
    }
    else
    {
        (void)OsaFilePrintF("DEBUG: OsaSocketSetAddress successful, osaErr: %d\n", osaErr);
        osaErr = OsaSocketConnect(ClientSocket, &remoteAddress);
    }
    if (osaErr != OSA_EOK)
    {
        // TODO: Create a generic log infrastructure later, CAT compliant (CO_Log_XYZ)
        (void)OsaFilePrintF("Error occurred while OsaSocketConnect, osaErr: %d\n", osaErr);
    }
    else
    {
        (void)OsaFilePrintF("DEBUG: OsaSocketConnect successful, osaErr: %d\n", osaErr);
        /*osaErr = OsaSocketSetOpt(ClientSocket, OSA_SOCKETOPTLVL_SOL, SO_SNDTIMEO,
            (char*)&SenderTimeOutInMs,
            sizeof(SenderTimeOutInMs));*/
    }

    if (osaErr != OSA_EOK)
    {
        // TODO: Create a generic log infrastructure later, CAT compliant (CO_Log_XYZ)
        (void)OsaFilePrintF("Error occurred while OsaSocketSetOpt, osaErr: %d\n", osaErr);
    }
    else
    {
        (void)OsaFilePrintF("DEBUG: OsaSocketSetOpt successful, osaErr: %d, strlen= %d\n", osaErr, (int)strlen(sendbuf));

        /*do
        {
            osaErr = OsaSocketSend(ClientSocket, &sendbuf, (int)strlen(sendbuf), 0, &BytesSent);
        } while (OSA_EOK == osaErr && (((int)strlen(sendbuf)) == BytesSent));*/
    }


    osaErr = OsaSocketSetOpt(ClientSocket, OSA_SOCKETOPTLVL_SOL, SO_RCVTIMEO, (char*)&ReceiveTimeOutInMs, sizeof(ReceiveTimeOutInMs));
    if (osaErr != OSA_EOK)
    {
        (void)OsaFilePrintF("Error occurred while setting socket options, osaErr: %d\n", osaErr);
    }


    //osaErr = OsaSocketRecv(ClientSocket, recvbuf, sizeof(recvbuf), 0, &BytesReceived);
    ////(void)OsaFilePrintF("Recv Buf:  %s\n", recvbuf);
    //(void)OsaFilePrintF("(Receiving String) Bytes Recevied:  %d\n", BytesReceived);
    //for (int i = 0; i < BytesReceived; i++)
    //{
    //	rawstring[i] = recvbuf[i];

    //}
    //rawstring[BytesReceived] = '\0';
    //(void)OsaFilePrintF("rawstring is:  %s\n", rawstring);

    /*Create a task to receive messages from the socket */
    /*if (OSA_EOK == osaErr)
    {

        osaErr = OsaThreadCreate("SocketReceiveTask", (OsaThreadStartFunc)&Receive_msg_socket2mbox_task, ClientSocket, RECEIVE_TASK_PRIORITY, RECEIVE_TASK_STACK_SIZE, &sck2mbxThreadId);
        (void)OsaFilePrintF("DEBUG: SocketReceiveTask Thread Created: osaErr %d\n", osaErr);
    }*/



    
    /* Create a task for command line interface*/
    
	
if (osaErr == OSA_EOK)
    {
        osaErr = OsaThreadCreate("CommandLine", (OsaThreadStartFunc)&CommandLineIntTask,
            ClientSocket, RECEIVE_TASK_PRIORITY2, RECEIVE_TASK_STACK_SIZE,
            &commandinterface);
        (void)OsaFilePrintF("DEBUG: Command Line Interface Created: osaErr %d\n", osaErr);
    }

	///*Create a task to send messages from the socket */
	if (OSA_EOK == osaErr)
	{
      osaErr = OsaThreadCreate("SocketSendTask", (OsaThreadStartFunc)&Send_msg_mbox2socket_task, ClientSocket, RECEIVE_TASK_PRIORITY, RECEIVE_TASK_STACK_SIZE, &mbx2sckThreadId);
		(void)OsaFilePrintF("DEBUG: SocketSendTask Thread Created: osaErr %d\n", osaErr);
	}
	while (1);
}

    /******************************************************************************/
                               /* Recieve message from socket*/
    /*****************************************************************************/ 
   /* void Receive_msg_socket2mbox_task(void)
    {
        osaErr = OsaSocketRecv(ClientSocket, recvbuf, sizeof(recvbuf), 0, &BytesReceived);
        (void)OsaFilePrintF("Receive_msg_socket2mbox_task Created \n");
        (void)OsaFilePrintF("(Receiving through Thread) Bytes Recevied:  %d\n", BytesReceived);
        for (int i = 0; i < BytesReceived; i++)
        {
            rawstring[i] = recvbuf[i];
        }
        rawstring[BytesReceived] = '\0';
        (void)OsaFilePrintF("rawstring is: %s \n", rawstring);
        
    }*/
        

   


/* Create a mutex*/
//void Initializemutex(void) 
//{
//	OsaErr osaErr = OSA_EOK;
//osaErr = OsaMutexCreate(OSA_SYNC_INVERSION_SAFE, &mutex);
//if (osaErr != OSA_EOK);
//	{
//		OsaFilePrintF("Mutex creation failed");
//	}
//}



/****************************************/
/* Commad Line Interface Taks           */
/****************************************/

void CommandLineIntTask(void)
{
	int index = 0;
	OsaErr   osaErr = OSA_ENOK;


	/*Print the CLI Banner */
	(void)OsaFilePrintF("/****************************************/\n");
	(void)OsaFilePrintF("/***     Command Line Interface       ***/\n");
	(void)OsaFilePrintF("/****************************************/\n");

	// InitTestMessageQ();

	forever
	{
		(void)OsaFilePrintF("\nEnter the Command string (Hit Enter once done, 20 char max)\n");
		for (index = 0; index < (MAX_CMD_STR_LEN - 1); index++)
		{
			cmdString[index] = getchar();
			if ('\n' == cmdString[index])
			{
				break;
			}
		}
		cmdString[index] = '\0';

		(void)OsaFilePrintF("\nCommand Entered is:\n");
		(void)OsaFilePrintF("%s\n", cmdString);
	}
	
}
  
/******************************************************************************/
							  /* Send message from socket*/
/*****************************************************************************/

void Send_msg_mbox2socket_task(void)
{
	OsaErr  osaErr = OSA_EOK;
	
	osaErr = OsaSocketSend(ClientSocket,(const CHAR*) cmdString, (int)strlen(cmdString), 0, &BytesSent);
	if (osaErr != OSA_EOK)
	{
		OsaFilePrintF("Send failed with error: %d\n", osaErr);
	}
	else
	{
		(void)OsaFilePrintF("Send_msg_mbox2socket_task Created \n");
		OsaFilePrintF("(Sending String through Thread) Bytes sent: %d\n", BytesSent);
		OsaFilePrintF("String is: %s\n", cmdString);
	}
	
}


























































    //recvbuf[BytesReceived] = '\0';
    
    /*if (BytesReceived > 0)
    {
        (void)OsaFilePrintF("String is:  %s\n", recvbuf);
        (void)OsaFilePrintF("string length:  %d\n", strlen(recvbuf));
    }*/
    
    /*i = strncpy_s(rawstring, sizeof(rawstring), recvbuf, sizeof(rawstring)-1);
    rawstring[sizeof(rawstring) - 1] = '\0';*/

    

    /*(void)OsaFilePrintF("BytesReceived: %d\n", BytesReceived);
    recvbuf[BytesReceived] = '\0';
    (void)OsaFilePrintF("BytesReceived: %d\n", recvbuf);
    for ( i = 0; recvbuf[i] != '\0'; i++)
    {
        recvbuf[i] = rawstring[i];
        rawstring[i] = '\0';
    }
    (void)OsaFilePrintF("String is: %s\n", rawstring);
    (void)OsaFilePrintF("No of character %d\n", i);*/






    /*if (osaErr != OSA_EOK)
    {
        (void)OsaFilePrintF("error:  %s\n",osaErr);
    }
    else
    {
        recvbuf[BytesReceived] = '\0';
        strcpy_s(msg, recvbuf);
        (void)OsaFilePrintF("string is:  %s\n", msg);*/


    

    

    /*if (BytesReceived < sizeof(recvbuf))
    {
        recvbuf[BytesReceived] = '\0';
        (void)OsaFilePrintF("String is: %s\n", recvbuf);
    }
    else
    {
        recvbuf[sizeof(recvbuf) - 1] = '\0'; 
        (void)osafileprintf("string is: %s\n", recvbuf);
    }*/


    
    //(void)OsaFilePrintF("Received Bytes: %d osaErr: %d\n", BytesReceived, osaErr);
    
    
    

    /*for (int i = 0; i < BytesReceived; i++)
    {
        (void)OsaFilePrintF("Bytes Received %d, osaErr %d", BytesReceived, osaErr);
        (void)OsaFilePrintF(" Received %d\n",recvbuf[i]);
    }
*/
    /*while(1)
    {
        osaErr = OsaSocketRecv(ClientSocket, &Bytes, sizeof(Bytes), 0, &BytesReceived);

        OsaFilePrintF("Bytes Received, osaErr,: %d, %d\n", BytesReceived, osaErr);
        OsaFilePrintF(" Received,%d\n", Bytes);

    }*/
        /*if (osaErr == OSA_EOK)
        {
            if (BytesReceived > 0)
            {
                recvbuf[BytesReceived] = '\0';
                (void)OsaFilePrintF("Bytes received: %d\n", recvbuf);
            }
            else
            {
                (void)OsaFilePrintF("No data received.\n");
                break;
            }
        }
        else
        {
            (void)OsaFilePrintF("Receive failed with error: %d\n", osaErr);
            break;
        }*/

        //} while (osaErr == OSA_EOK && BytesReceived > 0)







//	OsaSocketProtocol protocol;
//
//	/* ZeroMemory(&hints, sizeof(hints));*/
//	addr.family = AF_INET;
//	protocol = OSA_SOCKETTYPE_TCP; //SOCK_STREAM & IPPROTO_TCP 
//
//	OsaSocketAddr *result = NULL;
//	OsaSocketAddr *ptr = NULL;
//	
//
//	/* ZeroMemory(&hints, sizeof(hints));
//	hints.ai_family = AF_INET;
//	hints.ai_socktype = SOCK_STREAM;
//	hints.ai_protocol = IPPROTO_TCP;*/
//
//	/* struct addrinfo *result = NULL,
//	   *ptr = NULL,
//	   hints;
//	   const char *sendbuf = "this is a test";
//	   char recvbuf[DEFAULT_BUFLEN];
//	   int iResult;
//	   int recvbuflen = DEFAULT_BUFLEN;*/
//
//	 
//
//	/*// Resolve the server address and port
//	iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
//	if (iResult != 0) {
//		(void)OsaFilePrintF("getaddrinfo failed with error: %d\n", iResult);
//		WSACleanup();
//		return 1;
//	}*/
//
//	// Resolve the server address and port
//	if (OSA_EOK == osaErr)
//	{
//		IPAddr = "127.0.0.1";
//		clientport = 26395;
//		osaErr = OsaSocketSetAddress(&addr, IPAddr, clientport);
//		(void)OsaFilePrintF("Socket address set successfully, %d", osaErr);
//	}
//	else
//		{
//			(void)OsaFilePrintF("Error occurred while setting socket address, osaErr: %d", osaErr);
//		}
//
//	/* // Attempt to connect to an address until one succeeds
//	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) { */
//
//	// Attempt to connect to an address until one succeeds
//	for (ptr = result; ptr != NULL; ptr = ptr->address)
//	{
//
//		/*// Create a SOCKET for connecting to server
//		ClientSocket = Osasocketcreate(ptr->ai_family, ptr->ai_socktype,
//			ptr->ai_protocol);
//		if (ClientSocket == INVALID_SOCKET) {
//			(void)OsaFilePrintF("socket failed with error: %ld\n", WSAGetLastError());
//			OsaSocketCleanup();
//			return 1;
//		}*/
//
//
//		/*// Connect to server.
//	iResult = connect(ClientSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
//	if (iResult == SOCKET_ERROR)
//	{
//		(void)OsaFilePrintF("send failed with error: %d\n", WSAGetLastError());
//		closesocket(ClientSocket);
//		ClientSocket = INVALID_SOCKET;
//		continue;
//	}
//	break;
//	}*/
//
//	// Connect to server.
//		if (OSA_EOK == osaErr)
//		{
//			osaErr = OsaSocketConnect(ClientSocket, &addr);
//			(void)OsaFilePrintF("Connected to server!\n");
//		}
//		else
//		{
//			(void)OsaFilePrintF("DEBUG: Not Connected to server!\n");
//		}
//
//		/*freeaddrinfo(result);*/
//
//		/*if (ClientSocket == INVALID_SOCKET) 
//		{
//			(void)OsaFilePrintF("Unable to connect to server!\n");
//			OsaSocketCleanup();
//			return 1;
//		}*/
//
//		/*// Send an initial buffer
//		iResult = send(ClientSocket, sendbuf, (int)strlen(sendbuf), 0);
//		if (iResult == SOCKET_ERROR) {
//			(void)OsaFilePrintF("send failed with error: %d\n", WSAGetLastError());
//			closesocket(ClientSocket);
//			WSACleanup();
//			return 1;
//		}*/
//
//		// Send an initial buffer
//		if (OSA_EOK == osaErr)
//		{
//			do
//			{
//				osaErr = OsaSocketSend(ClientSocket, &sendbuf, (int)strlen(sendbuf), 0, &BytesSent);
//				(void)OsaFilePrintF("Sending Message!\n");
//			} while (OSA_EOK == osaErr && 5 == BytesSent);
//		}
//			
//		else 
//		{
//			(void)OsaFilePrintF("DEBUG: Sending failed with error: %d\n", osaErr);
//			OsaSocketClose(ClientSocket);
//			OsaSocketCleanup();
//			return 1;
//		}
//			
//
//		(void)OsaFilePrintF("Bytes Sent: %ld\n", sendbuf);
//
//		/*// shutdown the connection since no more data will be sent
//		iResult = Shutdown(ClientSocket, SD_SEND);
//		if (iResult == SOCKET_ERROR) {
//			(void)OsaFilePrintF("shutdown failed with error: %d\n", WSAGetLastError());
//			closesocket(ClientSocket);
//			WSACleanup();
//			return 1;
//		}*/
//
//		// shutdown the connection since no more data will be sent
//		if (OSA_EOK == osaErr)
//		{
//			osaErr = OsaSocketShutdown(ClientSocket, OSA_SD_SEND);
//			(void)OsaFilePrintF("Shutting Down: %d\n");
//		}
//
//		if (osaErr != OSA_EOK)
//		{
//			(void)OsaFilePrintF("DEBUG: Shutdown failed with error: %d\n", osaErr);
//			OsaSocketClose(ClientSocket);
//			OsaSocketCleanup();
//			return 1;
//		}
//
//		/*
//		// Receive until the peer closes the connection
//		do {
//
//			iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
//			if (iResult > 0)
//				(void)OsaFilePrintF("Bytes received: %d\n", iResult);
//			else if (iResult == 0)
//				(void)OsaFilePrintF("Connection closed\n");
//			else
//				(void)OsaFilePrintF("recv failed with error: %d\n", WSAGetLastError());
//
//		} while (iResult > 0);*/
//
//		// Receive until the peer closes the connection
//		if (osaErr == OSA_EOK)
//		{
//			do
//			{
//				osaErr = OsaSocketRecv(ClientSocket, recvbuf, strlen(recvbuflen), 0, &BytesReceived);
//				(void)OsaFilePrintF("Sending Message!\n");
//			} while (osaErr == OSA_EOK);
//		}
//		else
//		{
//			(void)OsaFilePrintF("DEBUG: Receving failed with error: %d\n", osaErr);
//	
//		}
//
//		(void)OsaFilePrintF("Bytes received: %s\n", recvbuf);
//	
//
//		/* // cleanup
//		closesocket(ClientSocket);
//		WSACleanup();
//		return 0;*/
//
//		// cleanup
//		OsaSocketClose(ClientSocket);
//		OsaSocketCleanup();
//		
//		
//	}
//}


