#include <ManageHeapMemory.h>
// #include <stdio.h>

#ifdef MATLAB
#include "mex.h"
#endif

void *ManageHeapMemory(void *pointer, int nItems, int itemSize)
{
    // The default behavior of this function is not to do anything at all. So
    // set the output pointer to be the same as the input
    void *output = pointer;

    // Trivial case is when the size of the datastructure we are assigning is
    // less than or equal to zero, or the user has specified that there are
    // negative items to allocate. Abort!
    if (itemSize <= 0 || nItems < 0) {
        return NULL;
    }

    int currentHeapSize = nItems * itemSize;
    int newHeapSize     = currentHeapSize + itemSize * HEAP_SIZE_INCREMENT;

    // If our current heap size is equal to 0, we need to allocate new memory.
    // We use calloc() (contiguous allocation) to allocate n blocks of size
    // itemSize bytes, where n = HEAP_SIZE_INCREMENT. The calloc() function 
    // will allocate this memory on the heap and set it to all zeros.
    //
    // Note: that behavior is unstable if you don't use the MATLAB versions for
    // MATLAB implementations. Boo!
    #ifdef MATLAB
    if (currentHeapSize == 0) {
        output = mxCalloc(HEAP_SIZE_INCREMENT, itemSize);  
        mexMakeMemoryPersistent(output);
    }
    #else
    if (currentHeapSize == 0) {
        output = calloc(HEAP_SIZE_INCREMENT, itemSize);
        // printf("calloc(%d, %d)\n", HEAP_SIZE_INCREMENT, itemSize);  // debug
    }
    #endif
    
    // If the number of items is *equal to* or *a multiple of* the number of
    // items in the array at pointer, then we need to increment the amount of
    // allocated memory so that we don't go over the allocated size.
    //
    // The `else if` here is important instead of just `if`, because in the 
    // singular case where `currentHeapSize == 0` (in the above condition),
    // then `nItems % HEAP_SIZE_INCREMENT == 0` (i.e. 0 mod something) will 
    // also equal zero! This causes a `realloc` to be run immediately following
    // the `calloc`, which is unnecessary.
    #ifdef MATLAB
    else if (nItems % HEAP_SIZE_INCREMENT == 0) {
        output = mxRealloc(pointer, newHeapSize);
        mexMakeMemoryPersistent(output);
    }
    #else
    else if (nItems % HEAP_SIZE_INCREMENT == 0) {
        output = realloc(pointer, newHeapSize);
        // printf("realloc(%p, %d)\n", pointer, newHeapSize);  // debug
    }
    #endif

    return output;
}