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

    Supersocket *s = mexGetSupersocketPointer(prhs[0]);
    char *name    = mxArrayToString(prhs[1]);

    DiscoverSocket(s, name);

    return;
}


static void ValidateInputs( int nlhs,       mxArray *plhs[],
                            int nrhs, const mxArray *prhs[])
{
  ConfirmNumInputs(nrhs, 2);
  ConfirmIsInt64(prhs[0], 0);  // Pointer to supersocket
  ConfirmIsString(prhs[1], 1); // Name of supersocket to discover
  
}
