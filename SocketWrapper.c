#include "SocketWrapper.h"
#include "ManageHeapMemory.h"
#include <poll.h> // for sturct pollfd
#include <errno.h>
#include <unistd.h> // For unlink(), write


/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
            

/**
@brief Populate a socketWrapper structure with initial values

The usual way to work with a SocketWrapper object is to initialize it using
this helper function. It takes care of the struct sockaddr_in and sockaddr_un
initializations, which streamlines and cleans code.

It's important to note that the SocketWrapper has a status value.  This status value
is UNINITIALIZED at the end of this function call. Further details in the .h file
*/
int PopulateSocketWrapper(SocketWrapper *sw,
							  char *name, 
							  char *ip,
							  int port,
							  int domain,
							  int type,
							  int flags)
{
	strcpy(sw->name, name);
	PopulateSockaddr_in(&sw->inetStruct, ip, port);
	PopulateSockaddr_un(&sw->unixStruct, name);
	sw->domain   = domain;
	sw->type     = type;
	sw->status   = SOCKETWRAPPER_STATUS_UNINITIALIZED;
	sw->flags    = flags;
	sw->socket   = -1;

	return 0;

}


/**
@brief Initializes a SocketWrapper function based on its existing values. 
This function takes the values already declared
in the SocketWrapper contents and then responds
to the declared variables in binding the sockets

It's important to note that the multicast behavior is different than the unicast
behvior. Specifically, if the MULTICAST flag is set, then you cannot CONNECT
to a socket. This is done on purpose. There is a check at the CONNECT 
stage of things.

*/
int InitializeSocketWrapper(SocketWrapper *sw)
{
	// Step 1: Create the socket! If this fails, abort.
	sw->socket = socket(sw->domain, sw->type, 0);
	if(sw->socket < 0)
	{
		DisplayError("[%s] Creating socket: %s", sw->name, strerror(errno));
		sw->status = SOCKETWRAPPER_STATUS_SOCKETERROR;
		return -1;
	}
	else
	{
		Display("Socket Creation '%s' OK: %d", sw->name, sw->socket);
	}

	// Step 2: Make this socket reusable. There's no obvious downside not to
	// reuse sockets. The only potential problem comes if the user decides
	// to hard-set two sockets with exactly the same IP and port. Otherwise,
	// for the most part it makes sense to actually do this.
	int enable = 1;
	if (setsockopt(sw->socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
	{
	    DisplayError("Setsockopt(SO_REUSEADDR) '%s' : %s", sw->name, strerror(errno));
		return -1;
	}

	// Step 3: Bind the socket, if the user desires it. 
	if(ParseFlags(sw->flags, BIND))
	{
		// Step 3a: Bind to an AF_UNIX socket. The first step is to see if the local file
		// exists already. If it does, then we unlink it. Failure to do so launches an error.
		// It appears that NIX systems keep records of which process is bound to socket files.
		// If the process terminates and the file exists, then no one else can bind to it, 
		// and no one can send messages to it either.

		// At the end of a successful bind, the status becomes INITIALIZED.
		if(sw->domain == AF_UNIX)
		{
			if(DoesFileExist(sw->unixStruct.sun_path))
		   		if(unlink(sw->unixStruct.sun_path))
		   		{
		   			DisplayError("Unlink AF_UNIX socket for '%s': %s", sw->name, strerror);
					return -1;
		   		}

			if(bind(sw->socket, (const struct sockaddr *) &sw->unixStruct, sizeof(struct sockaddr_un)) < 0)
			{
				PrintSockaddr_un(&sw->unixStruct);
				DisplayError("Bind AF_UNIX socket for '%s' (%d): %s", sw->name, sw->socket, strerror(errno));
				sw->status = SOCKETWRAPPER_STATUS_BINDERROR;
				return -1;
			}
			else
			{
				Display("Bind AF_UNIX Socket '%s' OK: %d", sw->name, sw->socket);
				sw->status = SOCKETWRAPPER_STATUS_INITIALIZED;
			}
		}
		
		// Step 3b: Bind to a AF_INET socket. This is pretty standard. 
		if(sw->domain == AF_INET)
		{
			if(bind(sw->socket, (const struct sockaddr *) &sw->inetStruct, sizeof(struct sockaddr_in)) < 0)
			{
				PrintSockaddr_in(&sw->inetStruct);
				DisplayError("Bind AF_INET '%s' socket %d: %s", sw->name, sw->socket, strerror(errno));
				sw->status = SOCKETWRAPPER_STATUS_BINDERROR;
				return -1;
			}
			else
			{
				Display("Bind AF_INET '%s' Socket OK: %d", sw->name, sw->socket);	
				sw->status = SOCKETWRAPPER_STATUS_INITIALIZED;
				PopulateSockaddr_inFromSocket(&sw->inetStruct, sw->socket);

			}

		}
	}
	
	// Step 4: Listen to the socket. This only makes sense for AF_INET sockets. However, no explicit
	// checking happens to enforce this! It'll just barf with standard DisplayError() issues.
	if(ParseFlags(sw->flags, LISTEN))
	{
		if(listen(sw->socket , NUM_SOCKSTREAM_LISTENERS) < 0)
		{
			DisplayError("Establishing listen for '%s' (%d): %s", sw->socket, strerror(errno));
			sw->status = SOCKETWRAPPER_STATUS_LISTENERROR;
			return -1;			
		}
		else
		{
			Display("Listening AF_INET '%s' Socket OK: %d", sw->name, sw->socket);	
			sw->status = SOCKETWRAPPER_STATUS_INITIALIZED;
		}
	}
	
	// Step 5: Multicast initialization. If we're binding to a socket that is in 
	// fact Multicast, then we need a specialized initialization procedure.
	if(ParseFlags(sw->flags, MULTICAST))
	{
		struct sockaddr_in *multiLocal = (struct sockaddr_in *) &sw->inetStruct;
		struct ip_mreq mreq;
		mreq.imr_multiaddr.s_addr = multiLocal->sin_addr.s_addr;         
		mreq.imr_interface.s_addr = htonl(INADDR_ANY); 
		if(setsockopt(sw->socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
		{
			DisplayError("Establishing multicast socket for '%s' (%d): %s", sw->socket, strerror(errno));
			return -1;
		}
	}
	
	// Step 6: Connect. This is used for both SOCK_DGRAM and SOCK_STREAM connections. 
	// At the end we have a connected socket! Huzzah.
	if(ParseFlags(sw->flags, CONNECT) && ParseFlags(sw->flags, MULTICAST) == 0)
	{
		if(sw->domain == AF_UNIX)
		{
			if(connect(sw->socket, (const struct sockaddr *) &sw->unixStruct, sizeof(struct sockaddr_un)) < 0)
			{
				PrintSockaddr_un(&sw->unixStruct);
				DisplayError("Connect AF_UNIX for '%s' socket %d: %s", sw->name, sw->socket, strerror(errno));
				sw->status = SOCKETWRAPPER_STATUS_CONNECTERROR;
				return -1;
			}
			else
			{
				Display("Connect AF_UNIX for '%s' Socket OK: %d", sw->name, sw->socket);
				sw->status = SOCKETWRAPPER_STATUS_INITIALIZED;
			}
		}

		if(sw->domain == AF_INET)
		{
			if(connect(sw->socket, (const struct sockaddr *) &sw->inetStruct, sizeof(struct sockaddr_in)) < 0)
			{
				PrintSockaddr_in(&sw->inetStruct);
				DisplayError("Connect AF_INET for '%s' socket %d: %s", sw->name, sw->socket, strerror(errno));
				sw->status = SOCKETWRAPPER_STATUS_CONNECTERROR;
				return -1;
			}
			else
			{
				Display("Connect AF_INET Socket '%s' OK: %d", sw->name, sw->socket);	
				sw->status = SOCKETWRAPPER_STATUS_INITIALIZED;
			}
		}
	}
	return 0;
}

/**
@brief Close the socket

Right now, the function simply closes the socket (of course, only if it's been initialized)
*/
int CloseSocketWrapper(SocketWrapper *sw)
{
	if(sw->socket != -1)
		close(sw->socket);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

/**
@brief Send data to a remote destination

This is the main workhorse function. Both the SendData and SendMessage functions
are wrappers for this function. Their goals are jus to populate a struct iovec 
that is used for sending data. This slows it down a tad, since there is a second
function call, but it also means that there is a single function that actually
does the work. We note that this function uses connected sockets for outgoing
messages. However, if we're working with SOCK_STREAM sockets, then at this
point we haven't connected to anything. This mean the function is responsible
for opening and closing the connection. 

Importantly, the behavior is different depending on whether you are
sending a unicast vs MULTICAST message. The socket is assumed NOT to be 
connected (since the logic was introduced in InitializeSocketWrapper)
so the function sendmsg() is used rather than writev.

*/

int SendIOvecToSocketWrapper(SocketWrapper *sw, struct iovec *data, int nVec, MessagingOptions *options)
{
	if(sw->socket == -1)
	{
		DisplayWarning("[%s] Attempting to write to uninitialized Socket. Aborting", sw->name);
		return -1;
	}


	// The behavior here is different depending on whether you're trying to send a multicast
	// packet or trying to send a unicast packet. In the multicast case, you cannot use
	// connect() and have predictable behavior. In the unicast case, you can send to a 
	// connected socket, which should be faster.

	if(ParseFlags(sw->flags, MULTICAST) == 1)
	{
		struct msghdr message 	= {0};
		message.msg_iov 		= data;
		message.msg_iovlen 		= nVec;
		message.msg_name 		= &sw->inetStruct;
		message.msg_namelen  	= sizeof(struct sockaddr_in);
		
		if (sendmsg(sw->socket, &message, 0) < 0)
		{
			DisplayWarning("[%s][Socket: %d]Socket: %d. ~~Multicast~~ Failed Sending message: %s. ", sw->name, strerror(errno));
			return -1;			
		}
		return 0;
	}


	// This SocketWrapper will not have a connected socket if it's going to SOCK_STREAM. Otherwise
	// it's assumed a connection exists for SOCK_DGRAM.
	if(ParseFlags(sw->flags, CONNECT) == 0)
	{
		const struct sockaddr *addr;
		int len;
		if (sw->domain == AF_UNIX)
		{
			addr = (const struct sockaddr*) &sw->unixStruct;
			len = sizeof(struct sockaddr_un);
		}
		else
		{
			addr = (const struct sockaddr*) &sw->inetStruct;
			len = sizeof(struct sockaddr_in);			
		}

		if(connect(sw->socket, addr, len) < 0)
		{
			DisplayWarning("Connect FAIL for '%s' socket %d: %s", sw->name, sw->socket, strerror(errno));
			return -1;
		}		
	}

	// Write the message to the socket! N.B. we're using connected sockets for SOCK_DGRAM.
	if(writev(sw->socket, data, nVec) < 0)
	{
		DisplayWarning("[%s][Socket: %d]Socket: %d. Failed Sending message: %s. ", sw->name, strerror(errno));
		return -1;
	}
	
	// If we're using SOCK_STREAM, close the connection please.
	if(sw->type == SOCK_STREAM)
	{
		close(sw->socket);
		sw->socket = socket(AF_INET, SOCK_STREAM, 0);
	}

	return 0;
}


int SendDataToSocketWrapper(SocketWrapper *sw, void *data, int dlen, MessagingOptions *options)
{
	struct iovec messageContents = {.iov_base = data, .iov_len =dlen};
	return SendIOvecToSocketWrapper(sw, (struct iovec*) &messageContents, 1, options);

}

int SendMessageToSocketWrapper(SocketWrapper *sw, Message *m)
{
	struct iovec messageContents[4] = {0};
	PopulateIOvec(messageContents, m);
	return SendIOvecToSocketWrapper(sw, (struct iovec*) &messageContents, 4, NULL);
}

/**

*/

int ReceiveIOvecFromSocketWrapper(SocketWrapper *sw, struct iovec *data, int nVec, MessagingOptions *options)
{
	int bytesRead = 0;
	if(sw->socket == -1 || sw->status == SOCKETWRAPPER_STATUS_UNINITIALIZED)
	{
		DisplayWarning("[%s] Attempting to receive from uninitialized Socket. Aborting", sw->name);
		return -1;
	}
	
	if(ParseFlags(sw->flags, BIND) == 0)
	{
		DisplayWarning("[%s] Attempting to read from unbound Socket. Aborting", sw->name);
		return -1;		
	}

	int readingSocket = sw->socket;

	if(ParseFlags(sw->flags, LISTEN))
	{
		int structLength = sizeof(struct sockaddr_in);
    	readingSocket = accept(sw->socket, (struct sockaddr *) &sw->inetStruct, (socklen_t*)&structLength);

	}

	bytesRead = readv(readingSocket, data, nVec);
	if(bytesRead < 0)
	{
		DisplayWarning("[%s] Failed Reading message: %s", sw->name, strerror(errno));
		return -1;
	}

	if(sw->type == SOCK_STREAM)
		close(readingSocket);

	return bytesRead;
}

int ReceiveDataFromSocketWrapper(SocketWrapper *sw, void *data, int dlen, MessagingOptions *options)
{
	struct iovec messageContents = {.iov_base = data, .iov_len = dlen};
	return ReceiveIOvecFromSocketWrapper(sw, (struct iovec*) &messageContents, 1, options);

}

int ReceiveMessageFromSocketWrapper(SocketWrapper *sw, Message *m)
{
	struct iovec messageContents[4] = {0};
	PopulateIOvec(messageContents, m);
	return ReceiveIOvecFromSocketWrapper(sw, (struct iovec*) &messageContents, 4, NULL);


}

int PopulateIOvec(struct iovec *messageContents, Message *m)
{
	messageContents[0].iov_base 	= m->from;
	messageContents[0].iov_len  	= PROCESS_MAX_CHARS;

	messageContents[1].iov_base 	= &m->id;
	messageContents[1].iov_len  	= sizeof(m->id);

	messageContents[2].iov_base 	= &m->dlen;
	messageContents[2].iov_len  	= sizeof(m->dlen);

	messageContents[3].iov_base 	= m->data;
	messageContents[3].iov_len  	= m->dlen;

	return 0;
}

int PollSocketWrapper(SocketWrapper *sw, int milliseconds)
{
	struct pollfd toPoll = {.fd = sw->socket, .events = POLLIN};
	int val = poll(&toPoll, 1, milliseconds);
	if (val < 0)
	{
		DisplayError("Could not poll bound sockets");
		return -1;	
	}

	return val;
}

int ReplyToSocketWrapper(int soc, SocketWrapper *sw, Message *m)
{
	struct iovec messageContents[4];
	PopulateIOvec(messageContents, m);
	struct msghdr messageHeader = {0};
	messageHeader.msg_name 			= &sw->inetStruct;
	messageHeader.msg_namelen 		= sizeof(struct sockaddr_in);
	messageHeader.msg_iov  		    = messageContents;
	messageHeader.msg_iovlen 		= 4;

	if(sendmsg(soc, &messageHeader, 0) < 0)
	{
		DisplayWarning("Failed Sending message: %s", strerror(errno));
		return -1;
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////


void PrintSocketWrapper(SocketWrapper *s)
{
	char domain[20] = {0};
	switch (s->domain)
	{
		case AF_INET : strcpy(domain, "AF_INET"); break;
		case AF_UNIX : strcpy(domain, "AF_UNIX"); break;
	}

	char type[20] = {0};
	switch (s->type)
	{
		case SOCK_DGRAM  : strcpy(type, "SOCK_DGRAM");  break;
		case SOCK_STREAM : strcpy(type, "SOCK_STREAM"); break;
	}

	char status[20] = {0};
	switch (s->status)
	{
		case SOCKETWRAPPER_STATUS_UNDEFINED     : strcpy(status, "Undefined"); 	    break;
		case SOCKETWRAPPER_STATUS_UNINITIALIZED : strcpy(status, "Uninitialized"); 	break;
		case SOCKETWRAPPER_STATUS_INITIALIZED   : strcpy(status, "Initialized"); 	break;
		case SOCKETWRAPPER_STATUS_SOCKETERROR   : strcpy(status, "ERROR: SOCKET"); 	break;
		case SOCKETWRAPPER_STATUS_BINDERROR     : strcpy(status, "ERROR: BIND"); 	break;
		case SOCKETWRAPPER_STATUS_CONNECTERROR  : strcpy(status, "ERROR: CONNECT"); break;
	}

    char flags[50] = {0};
	if(ParseFlags(s->flags, UNINITIALIZED))
		strcat(flags, "Uninitialized ");
	if(ParseFlags(s->flags, BIND))
		strcat(flags, "Bind ");
	if(ParseFlags(s->flags, CONNECT))
		strcat(flags, "Connect ");
	if(ParseFlags(s->flags, MULTICAST))
		strcat(flags, "Multicast ");	
	if(ParseFlags(s->flags, LISTEN))
		strcat(flags, "Listen ");		

	Display("Name      : %s", s->name);
	PrintSockaddr_in(&s->inetStruct);
	PrintSockaddr_un(&s->unixStruct);
	Display("Domain    : %s", domain);
	Display("Type      : %s", type);
	Display("Status    : %s", status);
	Display("Flags     : %s", flags);
	Display("Socket    : %d", s->socket);
}



void PrintSockaddr_in(struct sockaddr_in *addr)
{
    Display("AF_INET IP  : %s Port: %d", inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));  
}

void PrintSockaddr_un(struct sockaddr_un *addr)
{
    Display("AF_UNIX Path: %s",addr->sun_path);
}


/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

/**

*/
int AreSocketWrappersEqual(SocketWrapper *s1, SocketWrapper *s2)
{
	char *p1 = (char *) s1;
	char *p2 = (char *) s2;

	for (int i = 0; i < sizeof(SocketWrapper); i++)
		if(p1++ != p2++)
			return 0;

	return 1;
}

int PopulateSockaddr_in(struct sockaddr_in *addr, char *ip, int port)
{
	if(ip == NULL)
		return -1;
	
    addr->sin_family      = AF_INET;
    addr->sin_addr.s_addr = inet_addr(ip);
    addr->sin_port        = htons(port);

    return 0;

}

int PopulateSockaddr_inFromSocket(struct sockaddr_in *addr, int sock)
{
	socklen_t len = sizeof(struct sockaddr_in);
	if (getsockname(sock, (struct sockaddr *)addr, &len) == -1)
	{
    	DisplayError("Getsockname %d: ", sock);
		return -1;
	}

	return 0;
}

int PopulateSockaddr_un(struct sockaddr_un *addr, char *name)
{
	if(name == NULL)
		return -1;

    char    localSocketName[PROCESS_MAX_CHARS * 2] = {0};

    sprintf(localSocketName, AF_UNIX_FILENAME, name);

    addr->sun_family = AF_UNIX;

    strncpy(addr->sun_path, localSocketName, sizeof(addr->sun_path)-1);
	return 0;
}


int ParseFlags(int flags, Flag toParse)
{
	switch (toParse)
	{
		case UNINITIALIZED : return (flags >> 0) & 1; break;
		case BIND          : return (flags >> 1) & 1; break;
		case CONNECT       : return (flags >> 2) & 1; break;
		case MULTICAST     : return (flags >> 3) & 1; break;
		case LISTEN 	   : return (flags >> 4) & 1; break;
	}
	return -1;
}

/**
enum Flag {
	UNINITIALIZED, BIND, CONNECT, MULTICAST
}
ParseFlags (int flags, Flag f)  { // return the value of flag f in flagset 'flags'
   (flags >> (int f)) & 1
}
*/

int DoesFileExist(char *fname)
{
	return access( fname, F_OK ) != -1;
}



	// if(sw->socket == -1)
	// {
	// 	DisplayWarning("[%s] Attempting to write to uninitialized Socket. Aborting", sw->name);
	// 	return -1;
	// }
	
	// if(ParseFlags(sw->flags, CONNECT) == 0)
	// {
	// 	if(connect(sw->socket, (const struct sockaddr *) &sw->inetStruct, sizeof(struct sockaddr_in)) < 0)
	// 	{
	// 		DisplayWarning("Connect FAIL for '%s' socket %d: %s", sw->name, sw->socket, strerror(errno));
	// 		return -1;
	// 	}		
	// }

	// if(write(sw->socket, data, dlen) < 0)
	// {
	// 	DisplayWarning("[%s] Failed Sending message: %s", sw->name, strerror(errno));
	// 	return -1;
	// }

	// if(sw->type == SOCK_STREAM)
	// {
	// 	close(sw->socket);
	// 	sw->socket = socket(AF_INET, SOCK_STREAM, 0);
	// }

	// return 0;



	// if(sw->socket == -1)
	// {
	// 	DisplayWarning("[%s] Attempting to write to uninitialized Socket. Aborting", sw->name);
	// 	return -1;
	// }
	
	// if(ParseFlags(sw->flags, CONNECT) == 0)
	// {
	// 	if(connect(sw->socket, (const struct sockaddr *) &sw->inetStruct, sizeof(struct sockaddr_in)) < 0)
	// 	{
	// 		DisplayWarning("Connect FAIL for '%s' socket %d: %s", sw->name, sw->socket, strerror(errno));
	// 		return -1;
	// 	}		
	// }
	// struct iovec messageContents[4];
	// PopulateIOvec(messageContents, m);
	// struct msghdr messageHeader = {0};
	// messageHeader.msg_iov  		    = messageContents;
	// messageHeader.msg_iovlen 		= 4;

	// if(sendmsg(sw->socket, &messageHeader, 0) < 0)
	// {
	// 	DisplayWarning("[%s] Failed Sending message: %s", sw->name, strerror(errno));
	// 	return -1;
	// }

	// if(sw->type == SOCK_STREAM)
	// {
	// 	close(sw->socket);
	// 	sw->socket = socket(AF_INET, SOCK_STREAM, 0);
	// }



	// if(sw->socket == -1 || sw->status == SOCKETWRAPPER_STATUS_UNINITIALIZED)
	// {
	// 	DisplayWarning("[%s] Attempting to receive from uninitialized Socket. Aborting", sw->name);
	// 	return -1;
	// }
	
	// if(ParseFlags(sw->flags, BIND) == 0)
	// {
	// 	DisplayWarning("[%s] Attempting to read from unbound Socket. Aborting", sw->name);
	// 	return -1;		
	// }

	// int readingSocket = sw->socket;

	// if(ParseFlags(sw->flags, LISTEN))
	// {
	// 	int structLength = sizeof(struct sockaddr_in);
 //    	readingSocket = accept(sw->socket, (struct sockaddr *) &sw->inetStruct, (socklen_t*)&structLength);

	// }

	// if(read(readingSocket, data, dlen) < 0)
	// {
	// 	DisplayWarning("[%s] Failed Reading message: %s", sw->name, strerror(errno));
	// 	return -1;
	// }

	// if(sw->type == SOCK_STREAM)
	// 	close(readingSocket);

	// return 0;


	// if(sw->socket == -1 || sw->status == SOCKETWRAPPER_STATUS_UNINITIALIZED)
	// {
	// 	DisplayWarning("[%s] Attempting to receive from uninitialized Socket. Aborting", sw->name);
	// 	return -1;
	// }
	
	// if(ParseFlags(sw->flags, BIND) == 0)
	// {
	// 	DisplayWarning("[%s] Attempting to read from unbound Socket. Aborting", sw->name);
	// 	return -1;		
	// }


	// int readingSocket = sw->socket;

	// if(ParseFlags(sw->flags, LISTEN))
	// {
	// 	int structLength = sizeof(struct sockaddr_in);
 //    	readingSocket = accept(sw->socket, (struct sockaddr *) &sw->inetStruct, (socklen_t*)&structLength);

	// }

	// struct iovec messageContents[4];
	// PopulateIOvec(messageContents, m);
	// struct msghdr messageHeader = {0};
	// messageHeader.msg_iov  		    = messageContents;
	// messageHeader.msg_iovlen 		= 4;

	// //if(recvmsg(readingSocket, &messageHeader, 0) < 0)
	// if(readv(readingSocket, (const struct iovec*) &messageContents, 4) < 0)
	// {
	// 	DisplayWarning("[%s] Failed Reading message (%d): %s", sw->name, readingSocket, strerror(errno));
	// 	return -1;
	// }

	// if(sw->type == SOCK_STREAM)
	// 	close(readingSocket);

	// return 0;
