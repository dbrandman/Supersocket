#include "mex.h"
#include "Display.h"
#include "SocketWrapper.h"
#include "Supersocket.h"
#include "SupersocketListener.h"


void mexFunction( int nlhs,       mxArray *plhs[],
                  int nrhs, const mxArray *prhs[])
{
    int argc = 1;
    char *argv[1];
    argv[0] = "FILENAME";
    
    InitializeDisplay(argc, argv);
    SetColorfulness(DISABLE);
    SetVerbose(ENABLE);
    	
	SocketWrapper sw[4] = {0};
	PopulateSocketWrapper(&sw[0],"UnitTest1", "127.0.0.1", 5000, AF_INET, SOCK_DGRAM, BIND);
	PopulateSocketWrapper(&sw[1],"UnitTest2", "127.0.0.1", 5000, AF_INET, SOCK_DGRAM, CONNECT);
	PopulateSocketWrapper(&sw[2],"UnitTest1", NULL, 0, AF_UNIX, SOCK_DGRAM, BIND);
	PopulateSocketWrapper(&sw[3],"UnitTest1", NULL, 0, AF_UNIX, SOCK_DGRAM, CONNECT);
	
	for (int i = 0 ; i < 4; i++)
	{
		InitializeSocketWrapper(&sw[i]);
		Display("\n");
		PrintSocketWrapper(&sw[i]);
	}

	char dataToSend[] = "Message Sent From SocketWrapper";
	char buffer[200] = {0};



	Display("\n\nSending Data via AF_INET");
	SendDataToSocketWrapper(&sw[1], dataToSend, strlen(dataToSend), NULL);
	Display("Waiting to Receive...");
	ReceiveDataFromSocketWrapper(&sw[0], buffer, 200, NULL);
	Display("Received: %s", buffer);

	Display("\n\nSending Data via AF_UNIX");
	SendDataToSocketWrapper(&sw[3], dataToSend, strlen(dataToSend), NULL);
	Display("Waiting to Receive...");
	ReceiveDataFromSocketWrapper(&sw[2], buffer, 200, NULL);
	Display("Received: %s", buffer);

	Message m = CreateMessage(sw[1].name, 25, dataToSend, strlen(dataToSend));
	Message r = {0};
	r.dlen = 200;
	r.data = calloc(200,1);

	Display("\n\nSending Message via AF_UNIX");
	SendMessageToSocketWrapper(&sw[1], &m);
	Display("Waiting to Receive...");
	ReceiveMessageFromSocketWrapper(&sw[0], &r);
	PrintMessage(&r);

	Display("\n\nSending Message via AF_INET");
	SendMessageToSocketWrapper(&sw[3], &m);
	Display("Waiting to Receive...");
	ReceiveMessageFromSocketWrapper(&sw[2], &r);
	PrintMessage(&r);

	free(r.data);
    
    return;   
}