/**
 * @file
 * @authors David Brandman and Benjamin Shanahan
 * @brief Allocate and adjust heap memory at runtime.
 */

#ifndef __MANAGE_HEAP_MEMORY_H__
#define __MANAGE_HEAP_MEMORY_H__

#include <stdlib.h>

#define HEAP_SIZE_INCREMENT 10  ///< Increment multiplier for reallocation.

/**
 * Allocate and adjust heap memory at the given pointer at runtime. Please note
 * that this function is intended for step-wise, incrementing use-cases only!
 *
 * For example, **you WOULD use this function** if you have an array in heap 
 * memory that is being filled front-to-back, one element at a time. You would
 * call this function at each step of the way to determine if it is time to
 * `realloc` the array and provide more space to add additional elements to 
 * the end.
 *
 * As a counter-example, **you WOULD NOT use this function** if you have an
 * array that you want to resize on-demand. Let's say you have a pointer that 
 * points to heap memory and you wish to resize this. You are better off using
 * `realloc` on its own, as `ManageHeapMemory` makes assumptions about the 
 * order of the incoming queries. Basically, `ManageHeapMemory` assumes that it
 * is being run in a loop, so if you don't run it sequentially, providing all
 * possible values for `nItems`, it will never reach its `realloc` line if it
 * never sees `nItems` divisible by `HEAP_SIZE_INCREMENT`).
 *
 * Finally, be sure to `free` the pointers returned by this function when you
 * are finished with them. They will not be free'd otherwise!
 *
 * @param[in,out] pointer  Void pointer to memory location
 * @param[in]     nItems   Number of array elements at pointer
 * @param[in]     itemSize Size of individual array element in bytes
 *
 * Example usage:
 *
 * @code
    typedef struct {
        char one;
        char two;
    } MyStruct;
    MyStruct *myArray;

    // This emulates a step-wise, incrementing action. When j is zero, on the 
    // initial iteration, the `ManageHeapMemory` function call will return a
    // pointer to a space in heap-memory (using `calloc`). After j reaches the 
    // value of `HEAP_SIZE_INCREMENT`, the `myArray` pointer will be 
    // reallocated with an additional `HEAP_SIZE_INCREMENT` elements.
    for (int j = 0; j < 15; j++) {
        myArray = ManageHeapMemory(myArray, j, sizeof(MyStruct));
        myArray[j].one = j*2;
        myArray[j].two = j*2+1;
        // ...
    }
    free(myArray);
 * @endcode
 */
void *ManageHeapMemory(void *pointer, int nItems, int itemSize);

#endif  // end of include guard