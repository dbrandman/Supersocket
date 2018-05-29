#include "mex.h"
#include "Display.h"
#include "SocketWrapper.h"
#include "Supersocket.h"
#include "SupersocketListener.h"
#include "string.h"
#include "SupersocketMatlabHeader.h"

static void ValidateInputs( int nlhs,       mxArray *plhs[],
                            int nrhs, const mxArray *prhs[]);

void mexFunction( int nlhs,       mxArray *plhs[],
                  int nrhs, const mxArray *prhs[])
{
    INITIALIZE_DISPLAY

    ValidateInputs(nlhs, plhs, nrhs, prhs);

	Supersocket *s 	= mexGetSupersocketPointer(prhs[0]);
    char *name 		= mxArrayToString(prhs[1]);
    char *ip   		= mxArrayToString(prhs[2]);
    int port   		= (int) mxGetScalar(prhs[3]); 

   	InitializeSupersocket(s, name, ip, port);


    return;
}
static void ValidateInputs( int nlhs,       mxArray *plhs[],
                            int nrhs, const mxArray *prhs[])
{
	ConfirmNumInputs(nrhs, 4);
	ConfirmIsInt64(prhs[0], 0); // Pointer to supersocket
	ConfirmIsString(prhs[1], 1); // Name of supersocket
	ConfirmIsString(prhs[2], 2); // IP address
	ConfirmIsDouble(prhs[3], 3); // Port
}
