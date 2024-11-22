	// TestAppClient.cpp : Defines the entry point for the console application.
	//
	
	#define WIN32_LEAN_AND_MEAN
	
	#include "Osa.h"
	
	
	
	#define DEFAULT_PORT     26395  // --> "7015"
	// #define DEFAULT_PORT "26395"
	#define forever          for(;;)
	
	const CHAR* remoteIpAddress = " ";
	const UINT32   SenderTimeOutInMs = 2000;
	const UINT32   SenderFrequencyInMs = 500;
	const UINT32   ReceiveTimeOutInMs = 2000;
	const UINT32   ReceiveFrequencyInMs = 500;
	
	WSADATA wsaData;
	OsaSocket ClientSocket = INVALID_SOCKET;
	OsaSocketAddr remoteAddress;
	UINT16 remotePort = DEFAULT_PORT;
	INT32 bytesSent = 0;
	INT32 bytesReceived = 0;
	UINT8 bytes = 0;
	char rawstring[512];
	
	
	#define    DEFAULT_BUFLEN 512
	CHAR       sendbuf[DEFAULT_BUFLEN];
	CHAR       recvbuf[DEFAULT_BUFLEN];
	const CHAR *testMessage = "this is a test message from client";
	//INT32      recvbuflen = DEFAULT_BUFLEN;
	
	/* Thread related definitions */
	#define RECEIVE_TASK_STACK_SIZE        (12*1024)      
	#define RECEIVE_TASK_PRIORITY          60
	#define SEND_TASK_PRIORITY          60
	#define SEND_TASK_STACK_SIZE         (12*1024)
	#define COMMAND_TASK_PRIORITY        60
	#define COMMAND_TASK_STACK_SIZE       (12*1024)
	
	#define MIN_TRANSLATE_PRIORITY         196 
	
	static OsaThreadId  sck2mbxThreadId;
	void Receive_msg_socket2mbox_task(void);
	
	static OsaThreadId mbx2sckThreadId;
	void Send_msg_mbox2socket_task(void);
	
	/**** COMMAND LINE NTERFACE DEFINATIONS ******/
	static OsaThreadId commandinterface;
	void CommandLineIntTask(void);
	#define MAX_CMD_STR_LEN                   21
	static CHAR cmdString[MAX_CMD_STR_LEN] = { NULL };
	
	/***** MsgQ Definitions ******/
	void MsgQCreation(void);
	void MsgQSending(void);
	void MsgQRecv(void);
	#define max_msgs  10
	#define maxmsgsize 21
	const char* testqueue = "testqueue";
	OsaMsgQId Idtestqueue = NULL;
	//size_t* bytesRecv = 0;
	#define WORD_COUNT 21
	size_t sendbuflen = WORD_COUNT;
	UINT8 destinationbuf[WORD_COUNT];
	static UINT8 sourcebuffer[WORD_COUNT];
	static OsaThreadId MsgQRecvId;
	
	/****** Semaphore Definations ******/
	const UINT8 CLOSED_SEM = 0U;
	const UINT8 OPENED_SEM = 1U;
	const UINT8 BINARY_SEM = 1U;
	static OsaSyncId SemaphoreId = NULL;
	static OsaSyncId SendTaskSemaphoreId = NULL;
	
	
	/***** Mutex related definations****/
	OsaThreadId IdMutex1;
	OsaThreadId IdMutex2;
	OsaThreadId IdMutexThread;
	OsaSyncId MutexSync1 = NULL;
	OsaSyncId MutexSync2 = NULL;
	UINT8 count = 0;
	
	int main(int argc, char **argv)
	{
	OsaErr         osaErr = OSA_EOK;
	
	
	// OSA Initilization and socket startup
	OsaInitLib initLib = { NULL,
			   NULL,
			   TRUE,
			   TRUE };
	
	osaErr = OsaInit(&initLib);
	
	if (OSA_EOK == osaErr)
	{
	(void)OsaFilePrintF("OSA Inialization Sucessful!\n");
	}
	else
	{
	(void)OsaFilePrintF("OSA Inialization Failed! Error: %d\n", osaErr);
	}
	
	// Validate the parameters
	if (argc != 2) 
	{
	(void)OsaFilePrintF("usage: %s server-name\n", argv[0]);
	return(-1);
	}
	else
	{
	remoteIpAddress = argv[1];
	(void)OsaFilePrintF("DEBUG: argv[1]: %s, remoteIpAddress: %s\n", argv[1], remoteIpAddress);
	}
	
	/* Create a TCP Socket */
	if (OSA_EOK == osaErr)
	{
	osaErr = OsaSocketCreate(OSA_SOCKETTYPE_TCP, &ClientSocket);
	}
	if (osaErr != OSA_EOK)
	{
	// TODO: Create a generic log infrastructure later, CAT compliant (CO_Log_XYZ)
	(void)OsaFilePrintF("Error occurred while creating socket, osaErr: %d\n", osaErr);
	}
	else
	{
	(void)OsaFilePrintF("DEBUG: socket created successfully, osaErr: %d\n", osaErr);
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
	/*else
	{
	(void)OsaFilePrintF("DEBUG: OsaSocketConnect successful, osaErr: %d\n", osaErr);
	osaErr = OsaSocketSetOpt(ClientSocket, OSA_SOCKETOPTLVL_SOL, SO_SNDTIMEO,
		 (char*)&SenderTimeOutInMs,
		 sizeof(SenderTimeOutInMs));
	}
	*/
	//   if (osaErr != OSA_EOK)
	//   {
	//	// TODO: Create a generic log infrastructure later, CAT compliant (CO_Log_XYZ)
	//       (void)OsaFilePrintF("Error occurred while OsaSocketSetOpt, osaErr: %d\n", osaErr);
	//   }
	//   else
	//   {
	//       (void)OsaFilePrintF("DEBUG: OsaSocketSetOpt successful, osaErr: %d\n", osaErr);
	
	//	/*do
	//	{
	//		osaErr = OsaSocketSend(ClientSocket, &sendbuf, (int)strlen(sendbuf), 0, &BytesSent);
	//	} while (OSA_EOK == osaErr && (((int)strlen(sendbuf)) == BytesSent));*/
	//}
	
	
	
	osaErr = OsaSocketSetOpt(ClientSocket, OSA_SOCKETOPTLVL_SOL, SO_RCVTIMEO,
	     (char*)&ReceiveTimeOutInMs, 
	     sizeof(ReceiveTimeOutInMs));
	
	if (osaErr != OSA_EOK)
	{
	(void)OsaFilePrintF("Error occurred while setting socket options, osaErr: %d\n", osaErr);
	}
	else
	{
		/*Create a task to receive messages from the socket */
		osaErr = OsaThreadCreate("SocketReceiveTask",
			(OsaThreadStartFunc)&Receive_msg_socket2mbox_task,
			ClientSocket, RECEIVE_TASK_PRIORITY,
			RECEIVE_TASK_STACK_SIZE,
			&sck2mbxThreadId);
	
		(void)OsaFilePrintF("DEBUG: SocketReceiveTask Thread Created: osaErr %d\n", osaErr);
	}
	
	/*Create a task to send messages from the socket */
	if (OSA_EOK == osaErr)
	{
		//(void)OsaFilePrintF("DEBUG: SocketReceiveTask Thread Created: osaErr %d\n", osaErr);
	
		osaErr = OsaThreadCreate("SocketSendTask",
			(OsaThreadStartFunc)&Send_msg_mbox2socket_task,
			ClientSocket, SEND_TASK_PRIORITY,
			SEND_TASK_STACK_SIZE, &mbx2sckThreadId);
	
	
	
		if (OSA_EOK == osaErr)
		{
			(void)OsaFilePrintF("DEBUG: SocketSendTask Thread Created: osaErr %d\n", osaErr);
		}
		else
		{
			(void)OsaFilePrintF("SocketSendTask creation failed, osaErr: %d\n", osaErr);
		}
	}
	else
	{
		(void)OsaFilePrintF("SocketSendTask  Thread creation failed, osaErr: %d\n", osaErr);
	}
	
	
	/* Create a task for command line interface*/
	
	
	if (osaErr == OSA_EOK)
	{
		osaErr = OsaThreadCreate("CommandLine", (OsaThreadStartFunc)&CommandLineIntTask,
			ClientSocket, COMMAND_TASK_PRIORITY, RECEIVE_TASK_STACK_SIZE,
			&commandinterface);
		(void)OsaFilePrintF("DEBUG: Command Line Interface Created: osaErr %d\n", osaErr);
	}
	
	
	
	
	
	// Keep the application running
	forever;
	}
	
	/******************************************************************************/
	/*                  Recieve message from socket                               */
	/******************************************************************************/ 
	void Receive_msg_socket2mbox_task(void)
	{
	//OsaErr osaErr = OSA_EOK;
	//forever
	//{
	//    osaErr = OsaSocketRecv(ClientSocket, (CHAR*)recvbuf, sizeof(recvbuf), 0, &bytesReceived);
	//    if (osaErr != OSA_EOK)
	//    {
	//        OsaFilePrintF("Receive failed with error: %d\n", osaErr);
	//    }
	//    else
	//    {
	
	//       (void)OsaFilePrintF("(Receiving through Thread) Bytes Recevied:  %d\n", bytesReceived);
	//       for (int i = 0; i < bytesReceived; i++)
	//       {
	//            rawstring[i] = recvbuf[i];
	//       }
	//       rawstring[bytesReceived] = '\0';
	//       (void)OsaFilePrintF("rawstring is: %s \n", rawstring);
	//       
	//      // {
	//      //     (void)OsaFilePrintF("recvbuf[%d] = %d \n", i, recvbuf[i]);
	//      // }
	//    }
	//    OsaThreadSleep(500);
	//}
	//
	}
	
	/******************************************************************************/
	/*                 Send message from socket                                   */
	/******************************************************************************/
		
	
	void Send_msg_mbox2socket_task(void)
	{
		OsaErr osaErr = OSA_EOK;
	
		if (OSA_EOK == OsaSemTake(SendTaskSemaphoreId, OSA_INFINITE))
		{
			forever
			{
					osaErr = OsaMemCpy(sendbuf, (size_t)sizeof(sendbuf), sourcebuffer, (size_t)strlen(sourcebuffer));
					osaErr = OsaSocketSend(ClientSocket, sendbuf, (int)strlen(sendbuf), 0, &bytesSent);
					if (osaErr == OSA_EOK)
					{
							(void)OsaFilePrintF("\n Sending to server: %d\n", bytesSent);
					}
					else
					{
							(void)OsaFilePrintF("Sending to server failed \n ");
					}
	
	
			}
			OsaThreadSleep(1000);
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
	
		//InitTestMessageQ();
		MsgQCreation();
	
		
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
			
			
			MsgQSending();
			OsaSemGive(SemaphoreId);
			
			
			OsaThreadSleep(1000);
		}
			
		
	}
	
	/*****************************************************************************/
	
	
	void MsgQCreation(void)
	{
		OsaErr osaErr = OSA_EOK;
		if (Idtestqueue == NULL)
		{
	
			osaErr = OsaMsgQCreate(max_msgs, maxmsgsize, testqueue, &Idtestqueue);
			if (osaErr == OSA_EOK)
			{
				(void)OsaFilePrintF("MsgQ Created Successfully \n");
				//osaErr = OsaMsgQOpen(testqueue, &Idtestqueue);
				if (osaErr == OSA_EOK)
				{
					//(void)OsaFilePrintF("MsgQ Opened Successfully \n");
					osaErr = OsaSemCreate(CLOSED_SEM, BINARY_SEM, OSA_SYNC_FIFO, &SemaphoreId);
				}
				if (osaErr == OSA_EOK)
				{
					(void)OsaFilePrintF("Semaphore Created Successfully \n");
					osaErr = OsaSemCreate(OPENED_SEM, BINARY_SEM, OSA_SYNC_FIFO, &SendTaskSemaphoreId);
				}
				if (osaErr == OSA_EOK)
				{
					(void)OsaFilePrintF("Semaphore Send Task Created Successfully \n");
					osaErr = OsaThreadCreate("MsgQThread", (OsaThreadStartFunc)&MsgQRecv,
						NULL, RECEIVE_TASK_PRIORITY, RECEIVE_TASK_STACK_SIZE, &MsgQRecvId);
				}
				if (osaErr == OSA_EOK)
				{
					(void)OsaFilePrintF("Thread for MsgQ Created Successfully \n");
				}
			}
		}
	}
		
	
	
	void MsgQSending(void)
	{
		OsaErr   osaErr = OSA_EOK;
	
			if (OSA_EOK == OsaSemGive(SemaphoreId))
			{
				OsaMemCpy(sourcebuffer, (size_t)sizeof(sourcebuffer), cmdString, (size_t)sizeof(cmdString));
	
				osaErr = OsaMsgQSend(Idtestqueue, &sourcebuffer, (size_t)sizeof(sourcebuffer), OSA_INFINITE);
	
				if (OSA_EOK == osaErr)
				{
					(void)OsaFilePrintF("Message Sent to MsgQ: %s\n", sourcebuffer);
				}
			}
	
		
	}
	
	void MsgQRecv(void)
	{
	OsaErr   osaErr = OSA_EOK;
	UINT8    bytesReceived = 0;
	UINT8    i;
	
	forever
	{
	   if (OSA_EOK == OsaSemTake(SemaphoreId, OSA_INFINITE))
	   {
			   osaErr = OsaMsgQReceive(Idtestqueue, &sourcebuffer, WORD_COUNT,
						OSA_INFINITE, (size_t*)&bytesReceived);
	
			if (OSA_EOK == osaErr)
			{
				osaErr = OsaSemGive(SendTaskSemaphoreId);
				(void)OsaFilePrintF("\n Message Received: \n");
				for (i = 0; i < bytesReceived; i++)
				{
					(void)OsaFilePrintF("%c", sourcebuffer[i]);
				}
	
			}
	       
	}
		
		OsaThreadSleep(1500);
	}
	}
	
	
	
	
	
		
			
