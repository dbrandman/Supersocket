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

    Supersocket *s = mexGetSupersocketPointer(prhs[0]);
    void *data 		 = mxGetData(prhs[1]);
    int dlen   		 = (int) mxGetScalar(prhs[2]); 

    SendData(s, data, dlen, 0);


    return;
}


static void ValidateInputs( int nlhs,       mxArray *plhs[],
                            int nrhs, const mxArray *prhs[])
{
  ConfirmNumInputs(nrhs, 3);
  ConfirmIsInt64(prhs[0], 0); // Pointer to supersocket
  // NO CONFIRMATION FOR THE SECOND ENTRY. IT CAN BE ANYTHING
  ConfirmIsDouble(prhs[2], 2); // Data Length

  
}
