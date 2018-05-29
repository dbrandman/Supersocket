/**
@file
@brief Supersocket -- interprocess communication made easy

Supersocket is designed to make communication easy between processes on the same, or different computers.
The idea is that Supersocket is wrapper for a collection of SocketWrapper objects. A SocketWrapper is 
a structure that just makes all of the information pretaining to sockets very transparent and easy to use.

The way to think about a Supersocket is as a collection of different addresses that you would want to
send or receive messages from. For instance, the way to get started is by using:

@code
	Supersocket s = {0};
	AddSocket(&s, "Bob", "127.0.0.1", 5000, AF_INET, SOCK_DGRAM, CONNECT);

	char myData[] = "Hello, Bob!";
	Message m = CreateMessage("Alice", 0, dlen(myData), myData);

	SendMessage(&s, &m);
@endcode

Let's walk through this. The second line adds a SocketWrapper object to the Supersocket. Here,
we're addding the address of "127.0.0.1" port 5000, which we are naming Bob. We then create
a Message we are going to send Bob, (Message m). Finally, we call the SendMessage() function
which will then send m to Bob. See Message.h for information about Messages.

Suppose now we want to send a mesages to Bob and Charlie. Then we would go as follows:

@code
	Supersocket s = {0};
	AddSocket(&s, "Bob", "127.0.0.1", 5000, AF_INET, SOCK_DGRAM, CONNECT);
	AddSocket(&s, "Charlie", "127.0.0.1", 6000, AF_INET, SOCK_DGRAM, CONNECT);
	char myData[] = "Hello, Bob and Charlie!";
	Message m = CreateMessage("Alice", 0, dlen(myData), myData);

	SendMessage(&s, &m);	
@endcode

SendMessage() will send a message to both Bob and Charlie.

If you don't know where Bob and Charlie are located (i.e. don't have their IP and port
information, then take a look at SupersocketListener.h for information about how to
discover new addresses.

Suppose now we want to receive messages. Then we go as follows:

@code
	Supersocket s = {0};
	AddSocket(&s, "Alice", "127.0.0.1", 5000, AF_INET, SOCK_DGRAM, BIND);
	AddSocket(&s, "Alice", "127.0.0.1", 5000, AF_INET, SOCK_STREAM, BIND);
	AddSocket(&s, "Alice", "127.0.0.1", 5000, AF_INET, SOCK_DGRAM, BIND | MULTICAST);

	Message r = CreateMessageBuffer();
	ReceiveMessage(&s, &r);
@endcode

The call to ReceiveMessage() will actually poll all three sockets. Then, when a message
comes in to any of the sockets, it'll copy the data to Message r. 

@authors David Brandman and Benjamin Shanahan
*/

#pragma once

#include "SocketWrapper.h"
#include <pthread.h> // For thread locking
#include <poll.h> // for sturct pollfd

/**
@brief Definition of the Supersocket structure

A Supersocket contains an array of SocketWrapper structures. It also contains shortcuts
for which of the socketWrapper[] arrays is a bound socket and a connected socket,
in order to speed up the receiving and sending messages, accordingly. It also contains
a pthread_mutex_lock to prevent racing. This mostly comes into when SupersocketListener
wants to modify the Supersocket as its being used.
*/
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

/**
@brief InitializeSupersocket is a wrapper for a BIND call for AF_INET and AF_UNIX
*/
int InitializeSupersocket(Supersocket *s, char *name, char *ip, int port);

/**
 * @brief Close a Supersocket freeing heap memory and closing sockets as appropriate
 */
int CloseSupersocket(Supersocket *s);

/**
 * @brief Add a socket to the Supersocket
 *
 * Once a Supersocket is created, you should add sockets to the Supersocket. This is just
 * a wrapper for the PopulateSocketWrapper() and PopulateSocketWrapper() functions. 
 *
 * Returns the integer target of the newly added socket (i.e. for SendMessage).
 *
 * Actually, it turns out that this function is just a wrapper for the AddSocketWrapper() function
 * but the Devil's in the details.
 */

int AddSocket(Supersocket *s, char *name, char *ip, int port, int domain, int type, int flags);
/**
 * @brief Add a SocketWrapper structure to the Supersocket
 */ 
int AddSocketWrapper(Supersocket *s, SocketWrapper *sw);
/**
 * @brief Poll all of the SocketWrapper structures within the Supersocket
  Importantly, this function returns the number of polled sockets, as per the default behavior
  of the poll() function. So returning -1 is an error, returning 0 means no new messages, and
  the value represents how many new messages there are to read.  
*/
int PollSockets(Supersocket *s, int milliseconds);

/** Send data buffer to target. */
int SendData(Supersocket *s, int target, void *data, int dlen, MessagingOptions *options);
/** Send data buffer to all sockets in Supersocket. */
int SendDataToAll(Supersocket *s, void *data, int dlen, MessagingOptions *options);
/** Send message to target. */
int SendMessage(Supersocket *s, int target, Message *m);
/** Send message to all sockets in Supersocket. */
int SendMessageToAll(Supersocket *s, Message *m);


int ReceiveSupersocket(Supersocket *s, int receiveMessageFlag, Message *m, void *data, int dlen, MessagingOptions *options);
int ReceiveData(Supersocket *s, void *data, int dlen, MessagingOptions *options);
int ReceiveMessage(Supersocket *s, Message *m);
/**

*/
void PrintSupersocket(Supersocket *s);
void VerboseSupersocket(Supersocket *s);
