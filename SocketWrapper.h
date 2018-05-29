/**
@file
@brief Fundamental unit of the Supersocket system is a SocketWrapper

A Supersocket structure is composed of an array of SocketWrappers. The
SocketWrapper structure is the mainworkhorse. The idea is that a
SocketWrapper is just makes very explicit all of the normal baggage
that may be accessible after the `int val = socket()` function call.
By minimizing system calls, this speeds up the communication and also
makes things much more transparent for what's happening.

The usual procedure is to call the function as follows:

@code
	SocketWrapper s = {0};
	PopulateSocketWrapper();
	InitializeSocketWrapper();
@endcode

Suppose you wanted to listen to SOCK_DGRAM on port 5000. You'd go:

	PopulateSocketWrapper(&s, "Alice", "127.0.0.1", 5000, AF_INET, SOCK_DGRAM, BIND);

And if you wanted to bind to a multicast address, you'd go:

	PopulateSocketWrapper(&s, "Alice", "227.0.0.1", 5000, AF_INET, SOCK_DGRAM, BIND | MULTICAST);

And if you wanted to send a message to a SOCK_STREAM:

	PopulateSocketWrapper(&s, "Alice", "127.0.0.1", 5000, AF_INET, SOCK_STREAM, CONNECT );

And if you wanted to receive a message from a SOCK_STREAM:

	PopulateSocketWrapper(&s, "Alice", "127.0.0.1", 5000, AF_INET, SOCK_STREAM, BIND | LISTEN );

Once this is done you're probably interested in sending or receiving
messages using a SocketWrapper object. There are two wrappers to 
do this. There is a ReceiveMessage() and SendMessage() wrapper, which 
are designed to use the Message structure defined in Message.h. The
other way to use it is by using SendData and ReceiveData, which
are designed to work on bytestreams. The Message system makes it
really convenient to communicate the contents of a message being sent.

@authors David Brandman and Benjamin Shanahan
*/

#pragma once

#include <netinet/in.h>  // for struct sockaddr_in
#include <arpa/inet.h> // for the inet() call for populating structs and such
#include <sys/socket.h> // socket() creation
#include <sys/un.h> // for sturct_sockaddr_un
#include "Message.h"
#include "Display.h"



/**
* @brief Define the file path and name of a named socket in AF_UNIX

Note that named sockets in UNIX are used for the AF_UNIX communication
scheme. The AF_UNIX_FILENAME is populated at the time of calling
CreateAddress() and the %s field is populated by the name of the
address. This means that to communicate with Apple, a process
would connect to /tmp/p_Apple().

It turns out that AF_UNIX supports both SOCK_DGRAM and SOCK_STREAM.
However, from my research the SOCK_DGRAM is lossless, so it made
sense to just create the UDP communication method for this.
*/
#define AF_UNIX_FILENAME "/tmp/p_%s"  

/** 
@brief Define how many SOCK_STREAM listeneres there are on a single port
*/
#define NUM_SOCKSTREAM_LISTENERS 3


/**
@brief Definition of the SocketWrapper structure

The SocketWrapper has a name, which doesn't need to be unique within the Supersocket
infrastructure. This is usually defined using the Supersocket system. The
structure also carries with it the struct sockaddr_in and sockaddr_un, although
both are usually not initialized with meaningful data. 

The following fields refer to:

- **domain:** whether this is AF_INET or AF_UNIX
- **type:**   whether the socket is SOCK_DGRAM or SOCK_STREAM
- **status:** See the enum struct SOCKETWRAPPER_STATUS_LIST for more info
- **flags:**  See enum Flag for more info
- **socket:** contains the int corresponding to the file descriptor for the socket

Note: `PROCESS_MAX_CHARS` defined in Message.h
*/
typedef struct
{
	char name[PROCESS_MAX_CHARS];
	struct sockaddr_in inetStruct;
	struct sockaddr_un unixStruct;
	int domain; // AF_UNIX vs AF_INET
	int type; // SOCKET_DGRAM or SOCKET_STREAM
	int status; // Defines whether it is initialized, defined, offline, etc.
	int flags; // Defines whether it is connected or bound
	int socket; // Contains the binded / connected socket

} SocketWrapper;

/**
@brief List of statuses for a SocketWrapper

The definitions are as follows:
	
1. **Undefined:** This is the default value for when the socket is first created
				  using a SocketWrapper s = {0} call. This is done on purpose
				  since we want to be clear that there there isn't anything
				  in this SocketWrapper worth using here.

2. **Uninitialized:** This is the default value after caling PopulateSocketWrapper()
				      With this state the socket has been set to -1, which is not
				      a valid socket file descriptor and the functions will barf
				      if you try to send or receive messages.

3. **Initialized:** This is the value after a socket has been successfully declared.
 				    Note that nothing has to be bound or connected, just the socket
				    is valid and you can continue on your merry way. 

4. **Socketerror:** If you tried to create a socket and failed

5. **BindError:** If you tried to bind a socket and failed

6. **ConnectError:** If you tried to connect a socket and failed

7. **ListenError:** If you tried to listen to a socket and failed

*/
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
/**
@brief List of the different Flags for Populating / Initializing a SocketWrapper

This is a little bit not elegant, but it's very transparent as to what's happening.
These different flags are designed to be used with `|` (i.e. OR bit values) or added
together during declaration. So, for example, in the flags value of 
PopulateSocketWrapper() you would call `BIND | MULTICAST` if you wanted to
listen in on a multicast address. The values here are parse in the ParseFlags()
function.
*/
typedef enum 
{
	UNINITIALIZED 	= 1,
	BIND 			= 2,
	CONNECT 		= 4,
	MULTICAST 		= 8,
	LISTEN 		    = 16

} Flag;

/**
Right now this structure is empty, but it allows us to future proof a little bit
by saying that we may want options in the future!
*/
typedef struct 
{
	
} MessagingOptions;

/**
@brief Populate a SocketWrapper structure. See the above for the usual
way to call this function.

@code
	SocketWrapper s = {0};
	PopulateSocketWrapper(&s, "Alice", "127.0.0.1", 5000, AF_INET, SOCK_DGRAM, BIND);
@endcode


*/
int PopulateSocketWrapper(SocketWrapper *sw,
							  char *name, 
							  char *ip,
							  int port,
							  int protocol,
							  int type, 
							  int flags);

/**

*/
int InitializeSocketWrapper(SocketWrapper *sw);

/**

*/
int CloseSocketWrapper(SocketWrapper *sw);


/**

*/
int SendDataToSocketWrapper(SocketWrapper *sw, void *data, int dlen, MessagingOptions *options);
int ReceiveDataFromSocketWrapper(SocketWrapper *sw, void *data, int dlen, MessagingOptions *options);

/**
@brief Send a Message to a SocketWrapper

This function works as follows:
	1. Decide if this a valid SocketWrapper that can be used to send messages
	2. If (1), then write the contents to the socket with the message
*/
int SendMessageToSocketWrapper(SocketWrapper *sw, Message *m);

/**
@brief Receive Messages from a SocketWrapper

This function works as follows:
	1. Decide if this is a valid SocketWrapper that can be used to receive messages.
	2. If (1), then read the contents of the socket into the message
*/
int ReceiveMessageFromSocketWrapper(SocketWrapper *sw, Message *m);

/**
@brief Call poll() on a SocketWrapper with specified milliseconds value
*/
int PollSocketWrapper(SocketWrapper *sq, int milliseconds);

/**
@brief Decide if a .flags field contains a specific flag
The way to use this function is by providing the SOcketWrapper.flags entry
and also which flag you'd like to parse. So, if you want to know whether
this socket has the BIND flag, you would call it like:

@code
	int isBindTrue = ParseFlags(s.flags, BIND);
@endcode
*/
int ParseFlags(int flags, Flag toParse);

/**
@brief This is a helper function that is like a "reply." Here, we have the 
destination defined in the SocketWrapper sw, and we want to reply with
Message m. So we give the function an already created socket soc, and
the function will then send the contents of m to destination defined in sw. 

The usual use case for this is in the SupersocketListener function, when
we receive the return information from a different process, and that process
has supplied its return information as a SocketWrapper object. This function
makes it very clean and nice to send a reply message. 
*/
int ReplyToSocketWrapper(int soc, SocketWrapper *sw, Message *m);


/**
@brief Helper function to populate struct sockaddr_in
*/
int PopulateSockaddr_in(struct sockaddr_in *addr, char *ip, int port);
/**
@brief Helper function to populate sockaddr_in given an existing socket
*/
int PopulateSockaddr_inFromSocket(struct sockaddr_in *addr, int sock);

/**
@brief Helper function populate struct sockaddr_un
*/
int PopulateSockaddr_un(struct sockaddr_un *addr, char *name);

/**
@brief Helper function to print the contents of a SocketWrapper
*/
void PrintSocketWrapper(SocketWrapper *s);

/**
@brief Helper function to print struct sockaddr_in to the screen
*/
void PrintSockaddr_in(struct sockaddr_in *addr);

/**
@brief Helper function to print struct sockaddr_un to the screen
*/
void PrintSockaddr_un(struct sockaddr_un *addr);

int PopulateIOvec(struct iovec *messageContents, Message *m);

/**
@brief a short helper function that returns 1 if a file exists
and 0 if it doesn't.
*/
int DoesFileExist(char *fname);
