// TestAppServer.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"


#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "Osa.h"



// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")



#define DEFAULT_BUFLEN              512
 //#define DEFAULT_PORT             "7015"
#define DEFAULT_PORT                26395  // --> 7015

#define MAX_CLIENT_CONNECTIONS        2
#define MAX_SOCKET                    3
#define forever                  for(;;)

/* Thread related definitions */
#define RECEIVE_TASK_STACK_SIZE        (12*1024)      
#define RECEIVE_TASK_PRIORITY          60  
#define SEND_TASK_PRIORITY             60
#define MIN_TRANSLATE_PRIORITY         196      


static OsaThreadId  sck2mbxThreadId;
void Receive_msg_socket2mbox_task (void);

static OsaThreadId mbx2sckThreadId;
void Send_msg_mbox2socket_task(void);

static OsaThreadId cliThreadId;
void CommandLineIntTask (void);
#define MAX_CMD_STR_LEN                   21 
static CHAR cmdString[MAX_CMD_STR_LEN] = { NULL };

#define DEFAULT_BUFLEN 512
char recvbuf[DEFAULT_BUFLEN];
int recvbuflen = DEFAULT_BUFLEN;
INT32 BytesReceived = 0;
char rawstring[512];
char sparestring[20];



#ifdef WIN_NATIVE
SOCKET InitServerSocket(void);
int TestServerSocket(SOCKET clientFd);
#endif // 

// int TestServerSocketThread(OsaSocketAddr  clientAddr, OsaSocket  clientFd);

/*****************************************************************************/
/* stores the descriptor that can be used to send * /
/* messages to the specified package */
static OsaSocket              package_sockets[MAX_SOCKET];
static OsaSocket              accept_server_socket[MAX_SOCKET];

/* set txs as TRUE if pack server is started */
static volatile BOOLEAN       accept_server_started[MAX_SOCKET + 1];

static const CHAR* volatile   DefaultLocalAddress = "127.0.0.1";

// TODO: change it later based on total mumber of packages
static int                    packageID           = 0; 

/*****************************************************************************/
/* Semaphore Test Fucntions */
void TestSemaphore(void);
void SemTestTask1(void);
void SemTestTask2(void);

static OsaThreadId semTestTask1ID;
static OsaThreadId semTestTask2ID;

const  UINT8          BINARY_SEM     = 1U;
const  UINT8          OPENED_SEM     = 1U;
const  UINT8          CLOSED_SEM     = 0U;
static OsaSyncId      SemTask1Owner  = NULL;
static OsaSyncId      SemTask2Owner  = NULL;
static UINT16         testVar        = 0;

/*****************************************************************************/
/* Memory Alloc, Dealloc, Copy Test Fucntions */
void TestMemoryFunctions(void);

#define TEST_ALLOCATION_SIZE  21

static UINT8    sourceBuffer[TEST_ALLOCATION_SIZE]      = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
static UINT8    destinationBuffer[TEST_ALLOCATION_SIZE] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
UINT8*          srcPtr = NULL;
UINT8*          destPtr = NULL;

/*****************************************************************************/
/* String Test Fucntions */
void TestStringFunctions(void);


static UINT8    srcString1[] = "Source String 1";
static UINT8    srcString2[] = "Source String 1, Dest";

/*****************************************************************************/
/* Message Queue Test Fucntions */

#define MAX_NUM_MSGS    10
UINT32 maxmsgs = 50;
size_t buffersize[100];
const char* testqueue = "testqueue";
OsaMsgQId Idtestqueue = NULL;

void TestMessageQ(void);
void MQTestTask1(void);
void InitTestMessageQ(void);

const char*         TestQueueName1 = "TestMQOne_Test";
OsaMsgQId           IdTestQueueNameOne = NULL;
static OsaThreadId  MQTestTask1ID;
static OsaThreadId  MQTestTask2ID;
static UINT8        mqTestRecvBufferOne[TEST_ALLOCATION_SIZE];

/*****************************************************************************/
/* Message File Test Fucntions */

#define SIZE_OF_BUFFER           1000
#define NUM_OF_BYTES_TO_READ     20

void TestFileFunctions(UINT8* cmdString);

const char*         FullFilePathRead = "C:\\OsaFileTest\\TestFileRead.txt";
const char*         FullFilePathWrite = "C:\\OsaFileTest\\TestFileWrite.txt";
OsaFile             file_fd_rd = { 0 };
OsaFile             file_fd_wr = { 0 };
static UINT8        readbuffer[SIZE_OF_BUFFER];
static UINT8        writebuffer[SIZE_OF_BUFFER];
const char *sendbuf = "hi guys";

/*****************************************************************************/
OsaSocket      sockFd = OsaSocketInvalid;     /* socket file descriptor */
OsaSocket      sockOther = OsaSocketInvalid;  /* socket from accept */

OsaSocketAddr  serverAddr;    /* server's socket address */
OsaSocketAddr  clientAddr;    /* client's socket address */

OsaErr   osaErr = OSA_EOK;


INT32 BytesSent = 0;
const UINT32   SenderTimeOutInMs = 2000;
const UINT32   SenderFrequencyInMs = 500;

const char*         testQueue1 = "testQueue";
OsaMsgQId testQueueId = NULL;
void* destbuff[100];

int main(void)
{
	const CHAR* volatile local_ip_address = DefaultLocalAddress;
	

	// Initialize OSA
	OsaInitLib initLib = { NULL,
						  NULL,
						  TRUE,
						  FALSE };

	osaErr = OsaInit(&initLib);

	if (OSA_EOK == osaErr)
	{
		(void)OsaFilePrintF("OSA Inialization Sucessful!\n");
	}
	else
	{
		(void)OsaFilePrintF("OSA Inialization Failed! Error: %d", osaErr);
	}

	/* TODO:This is call needs to be part of the OSA Initalization process for Windows */
	/* Take care of this during refactoring */
	if (OSA_EOK == osaErr)
	{
		osaErr = OsaSocketStartup();
	}

	/* Create a TCP Socket */
	if (OSA_EOK == osaErr)
	{
		osaErr = OsaSocketCreate(OSA_SOCKETTYPE_TCP, &sockFd);
	}

	if (osaErr != OSA_EOK)
	{
		// TODO: Create a generic log infrastructure later, CAT compliant (CO_Log_XYZ)
		(void)OsaFilePrintF("Error occurred while creating socket, osaErr: %d", osaErr);
	}

	if (osaErr == OSA_EOK)
	{
		const INT32 soReuseAddrVal = 1;
		osaErr = OsaSocketSetOpt(sockFd, OSA_SOCKETOPTLVL_SOL, SO_REUSEADDR, (CHAR*)&soReuseAddrVal, sizeof(soReuseAddrVal));
		//osaErr = OsaSocketSetOpt(sockFd, OSA_SOCKETOPTLVL_TCP, TCP_NODELAY, (CHAR*)&soReuseAddrVal, sizeof(soReuseAddrVal));

		if (osaErr != OSA_EOK)
		{
			// TODO: Create a generic log infrastructure later, CAT compliant (CO_Log_XYZ)
			(void)OsaFilePrintF("Error occurred while setting socket options, osaErr: %d", osaErr);
		}
	}

	/* Fill in the socket structure */
	if (osaErr == OSA_EOK)
	{
		osaErr = OsaSocketSetAddress(&serverAddr, local_ip_address, (UINT16)DEFAULT_PORT);
		if (osaErr != OSA_EOK)
		{
			// TODO: Create a generic log infrastructure later, CAT compliant (CO_Log_XYZ)
			(void)OsaFilePrintF("Error occurred while setting socket address, osaErr: %d", osaErr);
		}
	}

	/* bind file descriptor to local address */
	if (osaErr == OSA_EOK)
	{
		osaErr = OsaSocketBind(sockFd, &serverAddr);

		while (osaErr == OSA_EPARAM)
		{
			/* Socket Address is not valid in its context. This is due to unavailable network media after system power-up */
			/* Retry until address is valid */
			OsaThreadSleep(1000);

			// TODO: Create a generic log infrastructure later, CAT compliant (CO_Log_XYZ)
			(void)OsaFilePrintF("Retry binding socket, osaErr: %d", osaErr);
			osaErr = OsaSocketBind(sockFd, &serverAddr);
		}
		if (osaErr != OSA_EOK)
		{
			(void)OsaFilePrintF("Error occurred while binding socket, osaErr: %d", osaErr);
			OsaSocketClose(sockFd);
		}
	}
	/* Queue creation for client connection requests */
	if (osaErr == OSA_EOK)
	{
		osaErr = OsaSocketListen(sockFd, MAX_CLIENT_CONNECTIONS);
		if (osaErr != OSA_EOK)
		{
			// TODO: Create a generic log infrastructure later, CAT compliant (CO_Log_XYZ)
			(void)OsaFilePrintF("Error occurred while listen to socket connection, osaErr: %d", osaErr);
			OsaSocketClose(sockFd);
		}
	}

	accept_server_socket[packageID] = sockFd;

	if (osaErr == OSA_EOK)
	{
		const INT32 tcpNoDelayVal = 1;
		(void)OsaSocketSetOpt(sockFd, OSA_SOCKETOPTLVL_TCP, TCP_NODELAY, (CHAR*)&tcpNoDelayVal, sizeof(tcpNoDelayVal));

		accept_server_started[packageID] = TRUE;

		/* accept new client connect requests */
		/* to make it thread that accepts new client connections make this a for-ever loop */
		/* forever */
		//{
		osaErr = OsaSocketAccept(sockFd, &clientAddr, &sockOther);

		// TODO: Create a generic log infrastructure later, CAT compliant (CO_Log_XYZ)
		(void)OsaFilePrintF("Here we go.... connection accepted\n");

		if (osaErr != OSA_EOK)
		{
			// TODO: Create a generic log infrastructure later, CAT compliant (CO_Log_XYZ)
			(void)OsaFilePrintF("Error occurred while accept socket connection, osaErr =%d", osaErr);

			OsaSocketClose(sockFd);
		}
		else
		{
			(void)OsaSocketSetOpt(sockOther, OSA_SOCKETOPTLVL_TCP, TCP_NODELAY, (CHAR*)&tcpNoDelayVal, sizeof(tcpNoDelayVal));
		}

		package_sockets[packageID] = sockOther;
		//}

		/* Create a task to send messages from the socket */
		/*if (OSA_EOK == osaErr)
		{
			osaErr = OsaThreadCreate("SocketSendTask", (OsaThreadStartFunc)&Send_msg_mbox2socket_task, sockOther, RECEIVE_TASK_PRIORITY, RECEIVE_TASK_STACK_SIZE, &mbx2sckThreadId);
			(void)OsaFilePrintF("DEBUG: SocketSendTask Thread Created: osaErr %d\n", osaErr);
		}*/

		/* Create a task to receive messages from the socket */
		if (OSA_EOK == osaErr)
		{
			osaErr = OsaThreadCreate("SocketReceiveTask", (OsaThreadStartFunc)&Receive_msg_socket2mbox_task, sockOther, RECEIVE_TASK_PRIORITY, RECEIVE_TASK_STACK_SIZE, &sck2mbxThreadId);
			(void)OsaFilePrintF("DEBUG: SocketReceiveTask Thread Created: osaErr %d\n", osaErr);
		}
	}

		///* Create a MsgQ*/
		//if (osaErr == OSA_EOK)
		//{
		//	OsaMsgQCreate(maxmsgs, buffersize, testqueue, &Idtestqueue);
		//	(void)OsaFilePrintF("DEBUG: MsgQ Created: osaErr %d\n", osaErr);
		//}

		///* sending through MsgQ*/
		//if (osaErr == OSA_EOK)
		//{
		//	osaErr = OsaMsgQSend(Idtestqueue, &rawstring, strlen(rawstring), OSA_INFINITE);
		//	(void)OsaFilePrintF("DEBUG: Sending through MsgQ: %s\n", osaErr);
		//}

		///* receving using MsgQ*/
		//size_t bytesrecv = 0;
		//if (osaErr == OSA_EOK)
		//{
		//	osaErr = OsaMsgQReceive(Idtestqueue, &destbuff, sizeof(destbuff), OSA_INFINITE, &bytesrecv);
		//	(void)OsaFilePrintF("DEBUG: Receving through MsgQ %s\n", destbuff);
		//}


		
	
		
		/*osaErr = OsaSocketSend(sockOther, sendbuf, (int)strlen(sendbuf), 0, &BytesSent);
		if (osaErr != OSA_EOK)
		{
			OsaFilePrintF("Send failed with error: %d\n", osaErr);
		}
		else
		{
			OsaFilePrintF("(Sending String) Bytes sent: %d\n", BytesSent);
			OsaFilePrintF("String is: %s\n", sendbuf);
		} */

		//if (OSA_EOK == osaErr)
		//{
		//	/* Create a task to receive messages from the socket */
		//	osaErr = OsaThreadCreate(
		//		"CliTask",
		//		(OsaThreadStartFunc)&CommandLineIntTask,
		//		NULL, /* pass the socket descriptor here*/
		//		RECEIVE_TASK_PRIORITY,
		//		RECEIVE_TASK_STACK_SIZE,
		//		&cliThreadId);
		//}
		


	

	//osaErr = OsaSocketSetOpt(sockOther, OSA_SOCKETOPTLVL_SOL, SO_SNDTIMEO, (char*)&SenderTimeOutInMs, sizeof(SenderTimeOutInMs));

	////if (osaErr == OSA_EOK)
	//{
	//	do
	//	{
	//		osaErr = OsaSocketSend(sockFd, &sendbuf, (int)strlen(sendbuf), 0, &BytesSent);

	//		if (osaErr == OSA_EOK)
	//		{
	//			OsaThreadSleep(SenderFrequencyInMs);
	//		}
	//	} while (((int)strlen(sendbuf)) == BytesSent);
	//}

	
	

	
#ifdef WIN_NATIVE
	SOCKET ClientSocket = InitServerSocket();
	TestServerSocket(SOCKET clientFd);
#endif
	forever;
	//(void)TestServerSocketThread(clientAddr, sockOther);
	return (osaErr);

}

/****************************************/
/* Receive Message from Socket Task     */
/****************************************/
void Receive_msg_socket2mbox_task(void)
{
	osaErr = OsaSocketRecv(sockOther, recvbuf, sizeof(recvbuf), 0, &BytesReceived);
	(void)OsaFilePrintF("Receive_msg_socket2mbox_task Created \n");
	(void)OsaFilePrintF("(Receiving through Thread) Bytes Recevied:  %d\n", BytesReceived);
	for (int i = 0; i < BytesReceived; i++)
	{
		rawstring[i] = recvbuf[i];
	}
	rawstring[BytesReceived] = '\0';
	(void)OsaFilePrintF("(rawstring is: %s \n", rawstring);
}
	///* Initial Test code created */
	//
	//for (int i = 0; i<5; i++)
	//{
	//    // (void)OsaFilePrintF("\n Receive_msg_socket2mbox_task Created %d\n", i);
	//}


	

	/******************************************************************************/
							   /* Send message from socket*/
	/*****************************************************************************/

void Send_msg_mbox2socket_task(void)
{
	osaErr = OsaSocketSend(sockOther, sendbuf, (int)strlen(sendbuf), 0, &BytesSent);
	if (osaErr != OSA_EOK)
	{
		OsaFilePrintF("Send failed with error: %d\n", osaErr);
	}
	else
	{
		(void)OsaFilePrintF("Send_msg_mbox2socket_task Created \n");
		OsaFilePrintF("(Sending String through Thread) Bytes sent: %d\n", BytesSent);
		OsaFilePrintF("String is: %s\n", sendbuf);
		
	}

}

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
		for (index = 0; index < (MAX_CMD_STR_LEN -1); index++)
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

		// TestSemaphore();
		// TestMemoryFunctions();
		// TestStringFunctions();
		// TestMessageQ();
		// TestFileFunctions(cmdString);

	}
}

/*****************************************************************************/

void TestSemaphore(void)
{
	OsaErr         osaErr = OSA_ENOK;
	
	/* Create a two tasks */
	osaErr = OsaThreadCreate(
		"SemTestTask1",
		(OsaThreadStartFunc)&SemTestTask1,
		NULL, /* pass the socket descriptor here*/
		RECEIVE_TASK_PRIORITY,
		RECEIVE_TASK_STACK_SIZE,
		&semTestTask1ID);

	if (OSA_EOK == osaErr)
	{
		osaErr = OsaThreadCreate(
			"SemTestTask2",
			(OsaThreadStartFunc)&SemTestTask2,
			NULL, /* pass the socket descriptor here*/
			RECEIVE_TASK_PRIORITY,
			RECEIVE_TASK_STACK_SIZE,
			&semTestTask2ID);
	}
	
	if (OSA_EOK == osaErr)
	{
		(void)OsaFilePrintF("\n Threads created for testing Semaphores\n");
		osaErr = OsaSemCreate(OPENED_SEM, BINARY_SEM, OSA_SYNC_FIFO, &SemTask1Owner);
		if (OSA_EOK == osaErr)
		{
			(void)OsaFilePrintF("\n Semaphore SemTask1Owner created\n");
		}
	}
}

void SemTestTask1(void)
{
	OsaErr         osaErr = OSA_ENOK;
	forever
	{
		osaErr = OsaSemTake(SemTask1Owner, OSA_INFINITE);
		if (OSA_EOK == osaErr)
		{
			testVar = testVar + 1;
			(void)OsaFilePrintF("\n SemTestTask1 has semphore, var increment by 1: testVar = %d", testVar);
		}

		OsaSemGive(SemTask1Owner);
	}
}

void SemTestTask2(void)
{
	OsaErr         osaErr = OSA_ENOK;
	forever
	{
		osaErr = OsaSemTake(SemTask1Owner, OSA_INFINITE);
		if (OSA_EOK == osaErr)
		{
			testVar = testVar + 1;
			(void)OsaFilePrintF("\n SemTestTask2 has semphore, var increment by 5: testVar = %d", testVar);
		}

		OsaSemGive(SemTask1Owner);
	}

}

/*****************************************************************************/

void TestMemoryFunctions(void)
{
	OsaErr         osaErr = OSA_ENOK;

	(void)OsaFilePrintF("\n Testing Memory allocation...");
	
	osaErr = OsaMemAlloc((void**)&srcPtr, TEST_ALLOCATION_SIZE);

	if (OSA_EOK == osaErr)
	{
		(void)OsaFilePrintF("\n Memory allocation successful, address srcPtr =0x%x", srcPtr);
	}

	osaErr = OsaMemCpy((void*)destinationBuffer, TEST_ALLOCATION_SIZE, (void*)sourceBuffer, TEST_ALLOCATION_SIZE);

	if (OSA_EOK == osaErr)
	{
		(void)OsaFilePrintF("\n Memory copy successful, copied buffer: \n");
		(void)OsaFilePrintF("destinationBuffer[] = ");
		for (int i = 0; i < TEST_ALLOCATION_SIZE; i++)
		{
			(void)OsaFilePrintF("%d, ", destinationBuffer[i]);
		}
		(void)OsaFilePrintF("\n");
	}
	
	osaErr = OsaMemCalloc((void**) &destPtr, TEST_ALLOCATION_SIZE, sizeof(UINT8*));
	if (OSA_EOK == osaErr)
	{
		osaErr = OsaMemCpy((void*)destinationBuffer, TEST_ALLOCATION_SIZE, (void*)destPtr, TEST_ALLOCATION_SIZE);
		if (OSA_EOK == osaErr)
		{
			(void)OsaFilePrintF("\n Memory After OsaMemCalloc Sucess: \n");
			(void)OsaFilePrintF("destinationBuffer[] = ");
			for (int i = 0; i < TEST_ALLOCATION_SIZE; i++)
			{
				(void)OsaFilePrintF("%d, ", destinationBuffer[i]);
			}
			(void)OsaFilePrintF("\n");
		}
	}

	osaErr = OsaMemFree((void**)&srcPtr);
	if (OSA_EOK == osaErr)
	{
		(void)OsaFilePrintF("\n Memory Free-ed successful, address srcPtr =0x%x\n", srcPtr);
	}
}

/*****************************************************************************/

void TestStringFunctions(void)
{
	OsaErr          osaErr = OSA_ENOK;
	static size_t   concatStrSize = 0;
	UINT8*          ptrDstString = NULL;


	(void)OsaFilePrintF("\n Testing String Operations...");


	concatStrSize = (sizeof(srcString2) + sizeof(srcString1));
	osaErr = OsaMemAlloc((void**)&ptrDstString, concatStrSize);

	if (OSA_EOK == osaErr)
	{
		osaErr = OsaMemCpy(ptrDstString, concatStrSize, srcString2, sizeof(srcString2));
		osaErr = OsaStrCat((CHAR*)ptrDstString, concatStrSize, (CHAR*)srcString1);

		if (OSA_EOK == osaErr)
		{
			(void)OsaFilePrintF("\n Size of concatStrSize = %d\n", concatStrSize);
			(void)OsaFilePrintF("\n Concatinated String: %s\n", ptrDstString);
		}

		osaErr = OsaMemFree((void**)&ptrDstString);

		if (OSA_EOK == osaErr)
		{
			(void)OsaFilePrintF("Memory Freed Sucessfully\n");
		}
	}
	
	
}

/*****************************************************************************/

void InitTestMessageQ(void)
{
	OsaErr   osaErr = OSA_ENOK;

	if (IdTestQueueNameOne == NULL)
	{
		osaErr = OsaMsgQCreate(MAX_NUM_MSGS,
			TEST_ALLOCATION_SIZE,
			TestQueueName1,
			&IdTestQueueNameOne);

		if (OSA_EOK == osaErr)
		{
			osaErr = OsaSemCreate(CLOSED_SEM, BINARY_SEM, OSA_SYNC_FIFO, &SemTask1Owner);
		}

		if (OSA_EOK == osaErr)
		{
			osaErr = OsaThreadCreate(
				"MQTestTask1",
				(OsaThreadStartFunc)&MQTestTask1,
				NULL, /* pass the socket descriptor here*/
				RECEIVE_TASK_PRIORITY,
				RECEIVE_TASK_STACK_SIZE,
				&MQTestTask1ID);

			if (OSA_EOK == osaErr)
			{
				(void)OsaFilePrintF("\n Thread-1 Spawnned for MQ testing\n");
			}
		}

	}
	
}

void TestMessageQ(void)
{
	OsaErr   osaErr = OSA_ENOK;

	if (OSA_EOK == OsaSemGive(SemTask1Owner))
	{
		OsaMemCpy(mqTestRecvBufferOne, (size_t)sizeof(mqTestRecvBufferOne), cmdString, (size_t)sizeof(cmdString));

		osaErr = OsaMsgQSend(IdTestQueueNameOne,
			&mqTestRecvBufferOne,
			(size_t)sizeof(mqTestRecvBufferOne),
			OSA_INFINITE);

		if (OSA_EOK == osaErr)
		{
			(void)OsaFilePrintF("Message Sent to MQ-1\n");
		}
	}

}

void MQTestTask1(void)
{
	OsaErr   osaErr = OSA_ENOK;
	UINT8    bytesReceived = 0;
	UINT8    index; 

	forever
	{
	   if (OSA_EOK == OsaSemTake(SemTask1Owner, OSA_INFINITE))
	   {
			osaErr = OsaMsgQReceive(IdTestQueueNameOne,
						&mqTestRecvBufferOne,
						TEST_ALLOCATION_SIZE,
						OSA_INFINITE,
						(size_t*)&bytesReceived);

			if (OSA_EOK == osaErr)
			{
				(void)OsaFilePrintF("\nMessage Received from Queue & Main Thread (bytesReceived = %d):\n", bytesReceived);
				(void)OsaFilePrintF("mqTestRecvBufferOne: ");
				for (index = 0; index < bytesReceived; index++)
				{
					(void)OsaFilePrintF("%c, ", mqTestRecvBufferOne[index]);
				}
				(void)OsaFilePrintF("\n");
			}
	   }

		OsaThreadSleep(10000);
	}
}

/*****************************************************************************/

void TestFileFunctions(UINT8* cmdString)
{
	OsaErr         osaErr = OSA_ENOK;
	size_t         actualBytesRead = 0;
	size_t         actualBytesWritten = 0;

	osaErr = OsaFileFOpen(&file_fd_rd, FullFilePathRead, "r");

	if (OSA_EOK == osaErr)
	{
		(void)OsaFilePrintF("\nTest file 1 opened in Read Mode\n");
		osaErr = OsaFileFRead(&readbuffer, sizeof(readbuffer), sizeof(UINT8), NUM_OF_BYTES_TO_READ, file_fd_rd, &actualBytesRead);
		OsaFilePrintF("\nContents of test File:\n");
		OsaFilePrintF("%s\n", readbuffer);
	}

	osaErr = OsaFileFOpen(&file_fd_wr, FullFilePathWrite, "w+");

	if (OSA_EOK == osaErr)
	{
		(void)OsaFilePrintF("\nTest file 2 opened in Wrtie & Read Mode\n");
		OsaMemCpy(&writebuffer, sizeof(writebuffer),cmdString, MAX_CMD_STR_LEN);

		osaErr = OsaFileFWrite(&writebuffer, sizeof(writebuffer), NUM_OF_BYTES_TO_READ, file_fd_wr, &actualBytesWritten);  
	}

	(void)OsaFileFClose(&file_fd_rd);
	(void)OsaFileFClose(&file_fd_wr);
}

/*****************************************************************************/

//int TestServerSocketThread(OsaSocketAddr  clientAddr, OsaSocket  clientFd)
//{
//	
//	char recvbuf[DEFAULT_BUFLEN];
//	int recvbuflen = DEFAULT_BUFLEN;
//
//	OsaErr         osaErr = OSA_EOK;
//	INT32          BytesReceived = 0;
//
//	/* Initialize windows socket */
//
//
//	/* Receive until the peer shuts down the connection */
//	forever
//	{
//
//		osaErr = OsaSocketRecv(clientFd, recvbuf, recvbuflen, 0, &BytesReceived);
//
//		if (osaErr != OSA_EOK)
//		{
//			(void)OsaFilePrintF("OsaSocketRecv ERROR: osaErr: %d\n", osaErr);
//			
//			// cleanup
//			//closesocket(clientFd);
//			//WSACleanup();
//			//break;
//		}
//		else
//		{
//			(void)OsaFilePrintF("Bytes received: %s\n", recvbuf);
//		}
//
//	}
//	return 0;
//}
