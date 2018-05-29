#include "mex.h"
#include "Display.h"

/*
 mex -DMATLAB GCC=/usr/bin/gcc-4.9 Test_Display.c -I../../../../Include/ -L../../../../Include LDFLAGS="\$LDFLAGS -Wl,-rpath/home/david/c/espa/Libraries/Include" -lsupersocket -L/usr/local/MATLAB/R2017b/sys/os/glnxa64/
   mex -v CFLAGS="\$CFLAGS -cxx -WI" LDFLAGS="\$LDFLAGS -rpath,/path/to/lib" mymexfile.c

 mex('-v','GCC=/usr/bin/gcc-4.9','Test_Display.c','-I../../../../Include/','-L../../../../Include','-L/usr/local/MATLAB/R2017b/sys/os/glnxa64/','LDFLAGS="\$LDFLAGS -Wl,-rpath,/home/david/c/espa/Include"', '-lsupersocket');

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
    
    Display("This is a number! %d", 5);
    Display("Nothing to format, just text!");
   
    FILE *log = fopen("test.txt","w");
    DisplayFile(log, "This is a line.");
    DisplayFile(log, "Another line.");
    fclose(log);
    
    return;   
} 