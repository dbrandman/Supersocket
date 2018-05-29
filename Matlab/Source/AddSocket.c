#include "mex.h"
#include "Display.h"
#include "SocketWrapper.h"
#include "Supersocket.h"
#include "SupersocketListener.h"
#include "string.h"
#include "SupersocketMatlabHeader.h"

static void ValidateInputs( int nlhs,       mxArray *plhs[],
                            int nrhs, const mxArray *prhs[]);
static int ParseCellFlags(const mxArray *flagCell);

void mexFunction( int nlhs,       mxArray *plhs[],
                  int nrhs, const mxArray *prhs[])
{
    INITIALIZE_DISPLAY

    ValidateInputs(nlhs, plhs, nrhs, prhs);

    Supersocket *s = mexGetSupersocketPointer(prhs[0]);


    char *name 		= mxArrayToString(prhs[1]);
    char *ip   		= mxArrayToString(prhs[2]);
    int port   		= (int) mxGetScalar(prhs[3]); 


    int domain;
    if(strcmp(mxArrayToString(prhs[4]), "AF_INET") == 0)
      domain = AF_INET;
    else
      domain = AF_UNIX;

    int type;
    if(strcmp(mxArrayToString(prhs[5]), "SOCK_DGRAM") == 0)
      type = SOCK_DGRAM;
    else
      domain = SOCK_STREAM;

    int flags = ParseCellFlags(prhs[6]);


    SocketWrapper sw = {0};
    PopulateSocketWrapper(&sw, name, ip, port, domain, type, flags);
    

    AddSocketWrapper(s, &sw);

    return;
}


static void ValidateInputs( int nlhs,       mxArray *plhs[],
                            int nrhs, const mxArray *prhs[])
{
  ConfirmNumInputs(nrhs, 7);
  ConfirmIsInt64(prhs[0], 0); // Pointer to supersocket
  ConfirmIsString(prhs[1], 1); // Name of supersocket
  ConfirmIsString(prhs[2], 2); // IP address
  ConfirmIsDouble(prhs[3], 3); // Port
  ConfirmIsString(prhs[4], 4); // Domain
  ConfirmIsString(prhs[5], 5); // Type
  ConfirmIsCell(  prhs[6], 6); // Flags
  
}

static int ParseCellFlags(const mxArray *flagCell)
{
  int output = 0;
  const mwSize *dims = mxGetDimensions(flagCell);

  for (mwIndex jcell = 0; jcell < dims[1]; jcell++) 
  {
      const mxArray *cellEntry = mxGetCell(flagCell,jcell);
      char *theString = mxArrayToString(cellEntry);
      if(strcmp(theString, "UNINITIALIZED") == 0) output += UNINITIALIZED;
      if(strcmp(theString, "BIND")          == 0) output += BIND;
      if(strcmp(theString, "CONNECT")       == 0) output += CONNECT;
      if(strcmp(theString, "MULTICAST")     == 0) output += MULTICAST;      
      if(strcmp(theString, "LISTEN")        == 0) output += LISTEN;
  }
  return output;
}