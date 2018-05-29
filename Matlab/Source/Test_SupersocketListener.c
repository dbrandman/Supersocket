

#include "SupersocketListener.h"
#include <stdio.h>
#include <unistd.h>
#include "Default_ESPA_Multicast.h"

#define USLEEPAMOUNT 1000000

/*
 mex -DMATLAB GCC=/usr/bin/gcc-4.9 Test_SupersocketListener.c CFLAGS="-std=c11 -fPIC" ../../../Display/Source/Display.c ../../../ManageHeapMemory/Source/ManageHeapMemory.c ../../Source/SocketWrapper.c ../../Source/SupersocketListener.c ../../Source/Message.c ../../Source/Supersocket.c -I../../../../Include/ -L/usr/local/MATLAB/R2017b/sys/os/glnxa64/
*/

void mexFunction( int nlhs,       mxArray *plhs[],
                  int nrhs, const mxArray *prhs[])
{
    int argc = 1;
    char *argv[1];
    argv[0] = "FILENAME";
    
    InitializeDisplay(argc, argv);
    SetColorfulness(DISABLE);
    SetVerbose(ENABLE);
	
	Supersocket alice 		= {0};
	Supersocket bob 		= {0};

	InitializeSupersocket(&alice, "Alice", "127.0.0.1", 5000);
	InitializeSupersocket(&bob, "Bob", "127.0.0.1", 5001);
	
	 InitializeSupersocketListener(&alice);
 	InitializeSupersocketListener(&bob);

 	char buffer[] = "THIS IS SOME TEXT!";
	 DiscoverSocket(&alice, "Bob");
	 Message m = CreateMessage("Alice", 0, buffer, strlen(buffer));
	 SendMessage(&alice, &m);

	 char buffer2[200] = {0};
	 Message r = {0};
	 r.dlen = 200;
	 r.data = buffer2;

	 ReceiveMessage(&bob, &r);
	 PrintMessage(&r);

	 PrintSupersocket(&alice);

	// usleep(USLEEPAMOUNT);

	// Supersocket sSend = {0};
	// AddSocket(&sSend, "Bob", DEFAULT_ESPA_MULTICAST_IP, DEFAULT_ESPA_MULTICAST_PORT, AF_INET, SOCK_DGRAM, CONNECT);

	// PrintSupersocket(&sSend);

	// SocketWrapper swSentAsData = {0};
	// PopulateSocketWrapper(&swSentAsData,  "Alice", NULL, 0, 0, 0, 0);

	// Message m = {0};
	// strcpy(m.from, "Bob");
	// m.id = ID_SUPERSOCKET_STRING_REQUEST_DISCOVERBIND;
	// m.dlen = sizeof(SocketWrapper);
	// m.data = &swSentAsData;

	// SendMessage(&sSend, &m);

	// usleep(USLEEPAMOUNT);

	Display("Push Enter...");
	getchar();
}