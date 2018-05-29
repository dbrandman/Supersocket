
#include "SupersocketMatlabHeader.h"

Supersocket *mexGetSupersocketPointer(const mxArray *m)
{
    IVP myValue = {0};
    void *vp;
    long long *abc;
    abc = (long long *) mxGetData(m);
    myValue.theInteger = *abc;

    Supersocket *s  = myValue.thePointer;

    return s;
}



void ConfirmNumInputs(int nrhs, int n)
{
    if(nrhs != n) 
      mexErrMsgIdAndTxt( "MATLAB:revord:invalidNumInputs",
              "Function requires %d inputs.", n); 	
}

void ConfirmAtLeastNumInputs(int nrhs, int n)
{
    if(nrhs != n) 
      mexErrMsgIdAndTxt( "MATLAB:revord:invalidNumInputs",
              "Function requires at least %d inputs.", n);	
}

void ConfirmIsInt64(const mxArray *d, int n)
{
    /* Must supply a scalar for input 2 */
    if (mxGetM(d)!= 1 || mxGetN(d)!= 1)
      mexErrMsgIdAndTxt( "MATLAB:revord:inputNotVector",
              "Input (%d) must be a scalar.", n);    
    
    /* Must supply a double for the second input */
    if( mxGetClassID(d) != mxINT64_CLASS)
      mexErrMsgIdAndTxt( "MATLAB:revord:inputNotDouble",
              "Input (%d) must be an Int64.", n);   	
}

void ConfirmIsDouble(const mxArray *d, int n)
{
    /* Must supply a scalar for input 2 */
    if (mxGetM(d)!= 1 || mxGetN(d)!= 1)
      mexErrMsgIdAndTxt( "MATLAB:revord:inputNotVector",
              "Input (%d) must be a scalar.", n);    
    
    /* Must supply a double for the second input */
    if( mxIsDouble(d) != 1)
      mexErrMsgIdAndTxt( "MATLAB:revord:inputNotDouble",
              "Input (%d) must be a double.", n);   	
}

void ConfirmIsString(const mxArray *d, int n)
{
    /* Input 1 must be a string */
    if ( mxIsChar(d) != 1)
      mexErrMsgIdAndTxt( "MATLAB:revord:inputNotString",
              "Input (%d) must be a string.", n);

}

void ConfirmIsCell(const mxArray *d, int n)
{
    /* Input 1 must be a string */
    if ( mxIsCell(d) != 1)
      mexErrMsgIdAndTxt( "MATLAB:revord:inputNotString",
              "Input (%d) must be a cell array.", n);

}