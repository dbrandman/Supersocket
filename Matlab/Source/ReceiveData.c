#include "mex.h"
#include "Display.h"
#include "SocketWrapper.h"
#include "Supersocket.h"
#include "SupersocketListener.h"
#include "string.h"
#include "SupersocketMatlabHeader.h"

#define DEFAULT_BUFFER_SIZE 2000

static void ValidateInputs( int nlhs,       mxArray *plhs[],
                            int nrhs, const mxArray *prhs[]);

void mexFunction( int nlhs,       mxArray *plhs[],
                  int nrhs, const mxArray *prhs[])
{
    INITIALIZE_DISPLAY

    ValidateInputs(nlhs, plhs, nrhs, prhs);

    Supersocket *s  = mexGetSupersocketPointer(prhs[0]);
    void *data      = mxCalloc(1, DEFAULT_BUFFER_SIZE); 

    int readSize = ReceiveData(s, data, DEFAULT_BUFFER_SIZE, 0);

    if(readSize < 0)
    {
      Display("Error reading! Aborting...");
      return;
    }



    plhs[0] = mxCreateNumericMatrix(1, readSize, mxUINT8_CLASS, mxREAL);

    data = mxRealloc(data, readSize);

    /* Point mxArray to dynamicData */
    mxSetData(plhs[0], data);
    return;
}


static void ValidateInputs( int nlhs,       mxArray *plhs[],
                            int nrhs, const mxArray *prhs[])
{
  ConfirmNumInputs(nrhs, 1);
  ConfirmIsInt64(prhs[0], 0); // Pointer to supersocket

  
}
