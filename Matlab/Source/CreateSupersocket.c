#include "mex.h"
#include "Display.h"
#include "SocketWrapper.h"
#include "Supersocket.h"
#include "SupersocketListener.h"
#include "string.h"
#include "SupersocketMatlabHeader.h"

void mexFunction( int nlhs,       mxArray *plhs[],
                  int nrhs, const mxArray *prhs[])
{
    INITIALIZE_DISPLAY

    IVP myValue = {0};
    myValue.thePointer = mxCalloc(1, sizeof(Supersocket)); 
    mexMakeMemoryPersistent(myValue.thePointer);


    plhs[0] = mxCreateNumericMatrix(1,1,mxINT64_CLASS,mxREAL);
    long long *ip;
    ip = (long long *) mxGetData(plhs[0]);
    *ip = myValue.theInteger;



    return;
}
