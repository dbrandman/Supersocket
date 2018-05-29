
#include "Supersocket.h"
#include <stdio.h>

/*
 mex -DMATLAB GCC=/usr/bin/gcc-4.9 Test_Supersocket.c CFLAGS="-std=c11 -fPIC" ../../../Display/Source/Display.c ../../../ManageHeapMemory/Source/ManageHeapMemory.c ../../Source/SocketWrapper.c ../../Source/Message.c ../../Source/Supersocket.c -I../../../../Include/ -L/usr/local/MATLAB/R2017b/sys/os/glnxa64/
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
	
	Supersocket sSend 		= {0};
	Supersocket sReceive 	= {0};

	InitializeSupersocket(&sReceive, "UnitTest", "127.0.0.1", 5000);

	AddSocket(&sReceive, "UnitTest1", "127.0.0.1", 5001, AF_INET, SOCK_DGRAM, BIND);
	AddSocket(&sReceive, "UnitTest2", "127.0.0.1", 5002, AF_UNIX, SOCK_DGRAM, BIND);
	AddSocket(&sReceive, "UnitTest2", "239.0.0.1", 5000, AF_INET, SOCK_DGRAM, BIND | MULTICAST);

	AddSocket(&sSend, "UnitTest", "127.0.0.1", 5000, AF_INET, SOCK_DGRAM, CONNECT);
	AddSocket(&sSend, "UnitTest", "239.0.0.1", 5000, AF_INET, SOCK_DGRAM, CONNECT);
	AddSocket(&sSend, "UnitTest", "127.0.0.1", 5001, AF_INET, SOCK_DGRAM, CONNECT);
	AddSocket(&sSend, "UnitTest", "127.0.0.1", 5001, AF_UNIX, SOCK_DGRAM, CONNECT);
	// Display("\n\n");
	// PrintSupersocket(&sReceive);

	// Display("\n\n");
	// PrintSupersocket(&sSend);

	char dataToSend[] = "My Message!";
	Message m = CreateMessage("UnitTest3", 25, dataToSend, strlen(dataToSend));
	Message r = {0};
	r.dlen = 200;
	r.data = calloc(200, 1);

	PrintSupersocket(&sReceive);
	PrintSupersocket(&sSend);

	SendMessage(&sSend, &m);
	
	Display("Waiting for Message 1...");
	ReceiveMessage(&sReceive, &r);
	VerboseSupersocket(&sReceive);
	PrintMessage(&r);
	VerboseSupersocket(&sReceive);
	
	Display("Waiting for Message 2...");
	ReceiveMessage(&sReceive, &r);
	VerboseSupersocket(&sReceive);
	PrintMessage(&r);
	VerboseSupersocket(&sReceive);

	Display("Waiting for Message 3...");
	ReceiveMessage(&sReceive, &r);
	VerboseSupersocket(&sReceive);
	PrintMessage(&r);
	VerboseSupersocket(&sReceive);

	Display("Waiting for Message 4...");
	ReceiveMessage(&sReceive, &r);
	VerboseSupersocket(&sReceive);
	PrintMessage(&r);
	VerboseSupersocket(&sReceive);


	return;
}