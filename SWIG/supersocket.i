/*
SWIG-Python Interface for Supersocket Library
---------------------------------------------

SWIG-Python interface file for Supersocket inter-process communications 
library.

Note: This wrapper is dependent on APIs defined in Superoskcet.h and parts of 
      the API defined in Display.h. If these APIs are altered, it is likely
      that it will break this SWIG interface.

31 January 2018, Benjamin Shanahan
*/


%pythonbegin %{
"""
Python Supersockets!

This extension allows Python to call functions from the Supersocket C library,
enabling interprocess communication within and throughout the ESPA 
infrastructure.

Here is an example usage, where we send a message to Supersocket Bob from 
Supersocket Alice:

    >>> from supersocket import *
    >>> alice = Supersocket()
    >>> bob = Supersocket()
    >>> InitializeSupersocket(alice, 'Alice', '127.0.0.1', 5001)
    >>> InitializeSupersocket(bob, 'Bob', '127.0.0.1', 5002)
    >>> InitializeSupersocketListener(alice)  # listen for discovery requests
    >>> InitializeSupersocketListener(bob)  # listen for discovery requests
    >>> DiscoverSupersocket(alice, 'Bob')  # tell Alice to find Bob
    >>> m = Message()
    >>> m._from = 'Alice'  # in Python, `from` is a reserved keyword
    >>> m.data = 'Hello, Bob!'
    >>> SendMessage(alice, m)
    >>> r = Message()
    >>> r.initialize(11)  # allocate and zero memory for incoming message
    >>> ReceiveMessage(bob, r)
    >>> print r, r.data

In this example, Supersockets Bob and Alice are both contained in the same 
process. This, however, is by no means required.

For more detailed examples and usage of Python supersocket, see the 
Supersocket/SWIG/test/ folder. In general, behavior and usage of this extension
is very similar to the C library. When in doubt, you can perform a Python 
`dir()` command on the current object to list all properties and methods 
associated with it. You can also use the `help()` command to print the
documentation associated with the given object or function.

NOTE: Since `from` is a reserved keyword in Python, this field in the Message
      object is replaced with `_from` (prepend an underscore).
"""
%}



///////////////////////////////////////////////////////////////////////////////
// Import SWIG interface dependencies.
///////////////////////////////////////////////////////////////////////////////

%module supersocket

// Tell SWIG to create some semblance of readable documentation
%feature("autodoc", "3");

// SWIG library dependencies
%include <stdint.i>
%include <cdata.i>
%include <typemaps.i>

// Python library dependencies. These will be incorporated into the 
%pythoncode %{
from struct import pack, unpack
%}



///////////////////////////////////////////////////////////////////////////////
// Define our interface.
///////////////////////////////////////////////////////////////////////////////

%{


#include "../Message.h"
#include "../SocketWrapper.h"
#include "../Supersocket.h"
#include "../SupersocketListener.h"
#include "../Display.h"


%}



///////////////////////////////////////////////////////////////////////////////
// Data structures and functions to expose from Supersocket
///////////////////////////////////////////////////////////////////////////////

/* From sys/socket.h and netinet/in.h */

enum {
    AF_UNIX     = 1, 
    AF_INET     = 2, 
    SOCK_STREAM = 1, 
    SOCK_DGRAM  = 2, 
    SOCK_RAW    = 3,
    IPPROTO_UDP = 17, 
    IPPROTO_TCP = 6, 
    INADDR_ANY  = 0
};


/* From Message.h */

typedef struct {
    char      from[PROCESS_MAX_CHARS];
    uint8_t   id;
    uint32_t  dlen;
    void     *data;
} Message;

Message CreateMessage(char *name, uint8_t id, void *data, uint32_t dlen);
Message CreateMessageBuffer(int bufferLength);
void DestroyMessageBuffer(Message *m);


/* From SocketWrapper.h */

typedef enum 
{
    SOCKETWRAPPER_STATUS_UNDEFINED,
    SOCKETWRAPPER_STATUS_UNINITIALIZED,
    SOCKETWRAPPER_STATUS_INITIALIZED,
    SOCKETWRAPPER_STATUS_SOCKETERROR,
    SOCKETWRAPPER_STATUS_BINDERROR,
    SOCKETWRAPPER_STATUS_CONNECTERROR,
    SOCKETWRAPPER_STATUS_LISTENERROR

} SOCKETWRAPPER_STATUS_LIST; 

typedef enum 
{
    UNINITIALIZED   = 1,
    BIND            = 2,
    CONNECT         = 4,
    MULTICAST       = 8,
    LISTEN          = 16

} Flag;


/* From Supersocket.h*/

typedef struct
{
    char name[PROCESS_MAX_CHARS];
    int nSockets;
    SocketWrapper *socketWrapper; 
    
    int nBoundSockets;
    int *boundSocketsList;
    struct pollfd *boundSocketsStruct;

    int nConnectedSockets;
    int *connectedSocketsList;

    pthread_mutex_t lock;

} Supersocket;

int InitializeSupersocket(Supersocket *s, char *name, char *ip, int port);
int CloseSupersocket(Supersocket *s);

int AddSocket(Supersocket *s, char *name, char *ip, int port, int domain, int type, int flags);
int AddSocketWrapper(Supersocket *s, SocketWrapper *sw);

int PollSockets(Supersocket *s, int milliseconds);

int SendData(Supersocket *s, int target, void *data, int dlen, MessagingOptions *options);
int SendDataToAll(Supersocket *s, void *data, int dlen, MessagingOptions *options);
int SendMessage(Supersocket *s, int target, Message *m);
int SendMessageToAll(Supersocket *s, Message *m);

int ReceiveSupersocket(Supersocket *s, int receiveMessageFlag, Message *m, void *data, int dlen, MessagingOptions *options);
int ReceiveData(Supersocket *s, void *data, int dlen, MessagingOptions *options);
int ReceiveMessage(Supersocket *s, Message *m);


/* From SupersocketListener.h */

int InitializeSupersocketListener(Supersocket *s);
void *SupersocketListener(void *);
int DiscoverSupersocket(Supersocket *s, char *name);



/* From Display.h */

enum Verbosity {
    DISABLE,
    ENABLE
};

int SetVerbose(int v);
int GetVerbose();
int SetFilename(char *newFilename);
int GetFilename();




///////////////////////////////////////////////////////////////////////////////
// Extend Message struct. This makes Message more class-like and allows us to
// include methods to help the user interact with fields that require pointers.
///////////////////////////////////////////////////////////////////////////////

%extend Message {

    ///////////////////////////////////////////////////////////////////////////
    // Class constructor and destructor.
    ///////////////////////////////////////////////////////////////////////////

    // Free any allocated memory in data field if it still remains. This gets
    // called when a Message object is deleted, either with the built-in `del`
    // function or if the Python interpreter is exited.
    ~Message() {
        if ($self->data != NULL) {
            free($self->data);
            $self->data = NULL;
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    // Wrap the Message struct's data field (which is a void pointer) so that 
    // users can easily set / get its value without worrying about pointers.
    ///////////////////////////////////////////////////////////////////////////

    // Return void pointer to data field.
    void *_get_data_c() {
        return $self->data;
    }

    // Wrap `get_data_c()` with a nice memory-access method provided by SWIG's
    // <cdata.i> library. See https://stackoverflow.com/a/10634684.
    %pythoncode %{
    def _get_data(self):
        return cdata(self._get_data_c(), self.dlen)
    %}

    // Set data field to char pointer and update dlen.
    void _set_data_c(char *data, int dlen) {
        $self->dlen = dlen;
        $self->data = malloc(dlen);
        memcpy($self->data, data, dlen);
    }

    // Wrap `set_data_c()` with a nice Python function.
    %pythoncode %{
    def _set_data(self, datastring):
        if type(datastring) != str:
            raise Exception("Data must be a string!")
        self._free_data_c()  # Free previously allocated data
        self._set_data_c(datastring, len(datastring))
    %}

    // Free allocated memory in data field.
    void _free_data_c() {
        if ($self->data != NULL) {
            free($self->data);
            $self->dlen = 0;
            $self->data = NULL;
        }
    }

    // Overwrite getter and setter methods for data field. This allows us to 
    // access the field like so:
    //
    //      >>> m.data = "something"
    //      >>> m.data
    //      'something'
    //
    // See https://stackoverflow.com/a/4750081/5161222.
    %pythoncode %{
        __swig_getmethods__["data"] = _get_data
        __swig_setmethods__["data"] = _set_data
        if _newclass: data = property(_get_data, _set_data)
    %}

    ///////////////////////////////////////////////////////////////////////////
    // Utility functions.
    ///////////////////////////////////////////////////////////////////////////

    %pythoncode %{
    def initialize(self, data_length):
        """
        Initialize data field of new Message object so that it has buffer 
        memory to receive an incoming message. 'Initialize' in this sense means
        to zero the memory after it has been allocated.
        """
        if data_length < 0:
            raise Exception("Cannot initialize negative bytes of memory!")
        self._free_data_c()
        if (data_length > 0):
            # Pack the data field with `data_length` zeros.
            self.data = pack("%ds" % data_length,"")
    %}

    char *__repr__() {
        static char tmp[1024] = {0};
        // There's a reason why we only print the pointer here and not the 
        // string (%s): since the data field doesn't need to be a string, but
        // merely an array of bytes, dlen is what is used to demarcate its end.
        // However, strings use a null character ('\0') to denote this, and so
        // more often than not, printing the contents at the data pointer will
        // result in an incomprehensible garbled mess.
        sprintf(tmp, \
            "Message:\n"
            "\t from: %s\n"
            "\t   id: %d\n"
            "\t dlen: %d\n"
            "\t*data: %p\n", \
            $self->from, $self->id, $self->dlen, $self->data);
        return tmp;
    }

    int __eq__(Message *other) {
        // return value of 0 means equal in Supersocket lib
        return AreMessagesEqual($self, other);
    }

}



/* Extend Supersocket struct to allow nice __repr__() and printing methods. */

%extend Supersocket {

    // Object 'representation' in Python console
    char *__repr__() {
        static char tmp[1024] = {0};
        sprintf(tmp, \
            "Supersocket:\n"
            "\t           Supersocket name: %s\n"
            "\t      SocketWrapper objects: %d\n" 
            "\t    Number of bound sockets: %d\n"
            "\tNumber of connected sockets: %d\n",  \
            $self->name, $self->nSockets, $self->nBoundSockets, $self->nConnectedSockets);
        return tmp;
    }

    // Wrap PrintSupersocket() so that user can access it.
    void Print() {
        int v = GetVerbose();
        int s = GetShowTrace();
        SetVerbose(ENABLE);
        SetShowTrace(DISABLE);

        // Print out the supersocket
        PrintSupersocket($self);
        
        SetVerbose(v);
        SetShowTrace(s);
    }

}