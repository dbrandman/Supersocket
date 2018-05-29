#include "mex.h"
#include "Supersocket.h"


#define INITIALIZE_DISPLAY \
    int argc = 1; \
    char *argv[1]; \
    argv[0] = "FILENAME"; \
    InitializeDisplay(argc, argv); \
    SetColorfulness(DISABLE); \
    SetVerbose(ENABLE); \
    SetMatlabMexPrintf(DISABLE); 

// usejava('Desktop') <<-- will be 0 if in no desktop mode. 

typedef union
{
    long long theInteger;
    void *thePointer;
} IVP;

Supersocket *mexGetSupersocketPointer(const mxArray *m);


void ConfirmNumInputs(int nrhs, int n);
void ConfirmAtLeastNumInputs(int nrhs, int n);
void ConfirmIsDouble(const mxArray *d, int n);
void ConfirmIsString(const mxArray *d, int n);
void ConfirmIsCell(const mxArray *d, int n);
void ConfirmIsInt64(const mxArray *d, int n);
