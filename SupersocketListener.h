/**
 * @file
 * @brief SupersocketListener is a tool for discovering other Supersockets within a network
 * 
 *
 *
 * This function is a handy tool for interprocess communication using the Supersocket structure.
 * Here's an example of how it's going to work:
 
	@code
	 	Supersocket s = {0};
		InitializeSupersocket("Alice", "127.0.0.1", 6000);
		InitializeSupersocketListener(&s);
		DiscoverSupersocket(&s, "Bob");
	@endcode

 * The call to InitializeSupersocket will generate a SOCK_DGRAM BIND address
 * at port 6000 for both AF_INET and AF_UNIX. Next, the InitializeSocketWrapper()
 * call launches a thread that sits and listens on a multicast address. This means
 * that the Supersocket s is now "Discoverable," so that other Supersockets
 * that want to discover "Alice" can now do so.
 *
 * The call to DiscoverSupersocket() will send out requests to find "Bob" on the network.
 * If Bob is not present, DiscoverSupersocket() will continue until "Bob" is found. If
 * Bob is present, the address with the name Bob will then reply to "Alice." Once
 * "Alice" receives Bob's information, it will get added as a CONNECT socket
 * to the Supersocket.
 *
 *
	
@authors David Brandman and Benjamin Shanahan
*/

#pragma once

#include "Supersocket.h"

/** Define how many microseconds between Multicast sends to find a Supersocket */
#define WAIT_TIME_BETWEEN_DISCOVERY_MULTICAST_REQUESTS 1000000 //microseconds

/** Define how long a process polls the BIND sockets before sending out another request */
#define POLL_TIME_FOR_DISCOVERY 500 //Milliseconds

/**
 * @brief List of the various ID for the Messages past between Supersocket Listeners
 * 
 * Here, all exchanges through the SupersocketListener will have a unique IDs.
 *
 * 
 * ID_SUPERSOCKET_SOCKETWRAPPER_REQUEST_DISCOVERBIND
 * 	
 * 	A message is being sent out, where any Listener that has a matching name
 * 	and a BIND, respond to the message
 *
 */
typedef enum 
{ 
	ID_SUPERSOCKET_UNDEFINED,
	ID_SUPERSOCKET_SOCKETWRAPPER_REQUEST_DISCOVERBIND,
	ID_SUPERSOCKET_SOCKETWRAPPER_REPLY_DISCOVERBIND,
	ID_SUPERSOCKET_SOCKETWRAPPER_DISABLE,
	ID_SUPERSOCKET_SOCKETWRAPPER_UDPATE,
	ID_SUPERSOCKET_SOCKETWRAPPER_CLOSE

} ID_SUPERSOCKETLISTENER_LIST;



/**
 * @brief Launch a thread that listens and responds to requests
*/
int InitializeSupersocketListener(Supersocket *s);

/**
 * @brief In case you don't want to launch a thread, you can call the function directly
 */
void *SupersocketListener(void *);
/**
 * @brief Discover another socket. Simply specific a name and it'll be added to the Supersocket
 */
int DiscoverSupersocket(Supersocket *s, char *name);
