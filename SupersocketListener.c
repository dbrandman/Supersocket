/**
 *@brief Definitions for using threads to communicate between sockets
 *
 * David Brandman and Ben Shanahan, 2018
 */
#include "SupersocketListener.h"
#include <unistd.h>
#include <errno.h>

static int InitializeMulticastSocketWrapper(SocketWrapper *sw, int flags);

static int ReplyToDiscoverBindRequest(int soc, SocketWrapper *multicastSocketWrapper, Supersocket *s, Message *incomingMessage);
// static int ParseDisable(Supersocket  *s, Message *incomingMessage); 
// static int ParseUpdate(Supersocket   *s, Message *incomingMessage);
// static int ParseClose(Supersocket    *s, Message *incomingMessage); 

#define DEFAULT_ESPA_MULTICAST_IP  	"239.0.0.1"
#define DEFAULT_ESPA_MULTICAST_PORT 5000




/** Call the supersocketListener() function as a thread */
int InitializeSupersocketListener(Supersocket *s)
{
    pthread_t supersocketListenerThread;
    pthread_create(&supersocketListenerThread, NULL, &SupersocketListener, s);

    return 0;
}

/** Main function for listening to requests from other procesess */
void *SupersocketListener(void *voidInput)
{
	// Cast the input as a Supersocket structure
	Supersocket *inputSupersocketPointer = (Supersocket *) voidInput;

	// Initialize a new Socketwrapper that we are going to be using to
	// listen to new messages. This is a multicast address
	SocketWrapper multicastSocketWrapper = {0};
	InitializeMulticastSocketWrapper(&multicastSocketWrapper, BIND | MULTICAST);
	
	// The ReplyToSocketWrapper() function requires an already created
	// socket explicitly for sending messages.
	int outgoingSocket = socket(AF_INET, SOCK_DGRAM, 0);
	
	// Initialize the message we are going to be writing to!
	Message incomingMessage = CreateMessageBuffer(1024);

	Display("[%s] Initializing Multicast Receiver...", inputSupersocketPointer->name);

	/** TODO: Turn this while(1) into while(enabled) based on a Status.h definition */

	while (1)
	{
		// We sit and block on multicastSocketWrapper until a new message arrives
		ReceiveMessageFromSocketWrapper(&multicastSocketWrapper, &incomingMessage);

		
		switch (incomingMessage.id)
		{
			case ID_SUPERSOCKET_SOCKETWRAPPER_REQUEST_DISCOVERBIND:

				ReplyToDiscoverBindRequest(outgoingSocket,
											&multicastSocketWrapper, 
											inputSupersocketPointer, 
											&incomingMessage); 
				break;

		}
	}
	CloseSocketWrapper(&multicastSocketWrapper);

}


static int ReplyToDiscoverBindRequest(int soc, SocketWrapper *multicastSupersocket, Supersocket *s, Message *incomingMessage)
{
	// This function responds to ID_SUPERSOCKET_SOCKETWRAPPER_REPLY_DISCOVERBIND as the ID input
	// from the Message. If that is the case, then it expects that the contents of the message
	// are a SocketWrapper, which contains:
	// 	1. The "name" field is the name of the process being requested
	// 	2. All other information within the SocketWrapper refers to how to reply to the sender

	// receivedSocketWrapper is the contents of the data field, which is the information required
	// for sending back a message
	SocketWrapper *receivedSocketWrapper = (SocketWrapper *) incomingMessage->data;
	char *nameRequested = receivedSocketWrapper->name;

	Display("[%s] Received Request for %s", s->name, nameRequested);

	// Now we iterate through each of the sockets that are currently bound on Supersocket s.
	// A match occurs if:
	// 	1. The requested name matches the name of the socketWrapper
	// 	2. The domain is AF_INET. This is because the discovery is assumed to work on AF_UNIX.
	
	
	for(int i = 0; i < s->nBoundSockets; i++)
	{
		int socketWrapperIndex 		= s->boundSocketsList[i];

		char *thisSocketWrapperName = s->socketWrapper[socketWrapperIndex].name;
		int thisSocketWrapperDomain = s->socketWrapper[socketWrapperIndex].domain;
		
		int condition1 = (strcmp(nameRequested, thisSocketWrapperName) == 0);
		int condition2 = (thisSocketWrapperDomain == AF_INET);
		
		if (condition1 && condition2)
		{
			// If a match occurs, then we are going to reply using the SocketWrapper information
			// from the match! We populate our reply message as follows:
			Display("[%s] I am %s! Replying to [%s]...", s->name, nameRequested, incomingMessage->from);

			Message replyMessage = CreateMessage(thisSocketWrapperName, 
					ID_SUPERSOCKET_SOCKETWRAPPER_REPLY_DISCOVERBIND ,
					&s->socketWrapper[socketWrapperIndex], 
					sizeof(SocketWrapper));


			ReplyToSocketWrapper(soc, receivedSocketWrapper, &replyMessage);

			return 0;
		}
	}
	return 1;
}


/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

int DiscoverSupersocket(Supersocket *s, char *name)
{
	// We first create a SocketWrapper that we are going to use for sending
	// messages out on the multicast. This is the multicastSocketWrapper
	//
	// Next we are going to create a socketWrapper that we are going to BIND
	// and listen to. This is the dataPayload. It's called this because
	// it's exactly the same SocketWrapper that is going to be sent out
	// on the Multicast network as the .data portion of the Message
	SocketWrapper multicastSocketWrapper = {0};
	InitializeMulticastSocketWrapper(&multicastSocketWrapper, CONNECT | MULTICAST);

	SocketWrapper dataPayload = {0};
	PopulateSocketWrapper(&dataPayload, 
						  name, 
						  DEFAULT_ESPA_MULTICAST_IP,
						  0,
						  AF_INET,
						  SOCK_DGRAM,
						  BIND | MULTICAST);
	InitializeSocketWrapper(&dataPayload);

	// messageRequest is the message sent out to the Network.
	// messageReply is a buffer fora new message we will be receiving
	Message messageRequest 	= CreateMessage(s->name, ID_SUPERSOCKET_SOCKETWRAPPER_REQUEST_DISCOVERBIND, &dataPayload, sizeof(SocketWrapper));
	Message messageReply 	= CreateMessageBuffer(2000);

	/* Display("MESSAGE REQUEST:"); */
	/* PrintMessage(&messageRequest); */


	int nMessages = 0;
	int socketFound = 0;
	int integerOfNewSocketWrapper;
	
	while (socketFound == 0)
	{
		// We begin by sending out a mesage on the Multicast network contaiing messageRequest
		Display("[%s] Sending out a request for: %s", s->name, name);
		SendMessageToSocketWrapper(&multicastSocketWrapper, &messageRequest);

		// Now we sit and poll whether there are any new messages. We timeout based on
		// the definition set in SupersocketListener.h
		// When there is at least one new message, we parse it. If it matches the
		// ID_SUPERSOCKET_SOCKETWRAPPER_REQUEST_DISCOVERBIND then we check to see if it's
		// in fact the name we want. If so, then we add it to the Supersocket s and then 
		// exit this function.
		// If we continue not receiving messages, then the nMessages == 0 and then we exit
		// the innter while() loop, which means we then resend the message to the network
		//
		// One of the things to consider is if there is a lot of traffic on the network,
		// then nMessages will constantly not be 0... Worth thinking about this a bit
		// TODO: Consider changing this code in case of busy multicast traffic
		while((nMessages = PollSocketWrapper(&dataPayload, POLL_TIME_FOR_DISCOVERY)) != 0)
		{
			if(nMessages < 0)
			{
				DisplayError("[%s] Unable to poll SocketWrapper: %s",  s->name, strerror(errno));		
				return -1;
			}	

			ReceiveMessageFromSocketWrapper(&dataPayload, &messageReply);
			if(messageReply.id == ID_SUPERSOCKET_SOCKETWRAPPER_REPLY_DISCOVERBIND)
			{
				Display("[%s] Received a reply to the Discovery from %s", s->name, messageReply.from);
				SocketWrapper *sw = messageReply.data;

				/* PrintMessage(&messageReply); */

				if(strcmp(sw->name, name) == 0)
				{
					// If we've come this far, then we are going to add the new information
					// to the Supersocket. At this point we check if the other process
					// is local or not, by asking if the file exists at the expected AF_UNIX
					// address, as defined by SocketWrapper.h
					sw->domain = DoesFileExist(sw->unixStruct.sun_path) ? AF_UNIX : AF_INET;
					sw->flags  = CONNECT;
					Display("[%s] Attempting to add %s to the Supersocket!", s->name, sw->name);
					integerOfNewSocketWrapper = AddSocketWrapper(s, sw);
					Display("[%s] Added %s to the Supersocket!", s->name, sw->name);

					socketFound = 1; // This will cause the outer while loop to break
					break;
				}
			}					
		}
	}


	DestroyMessageBuffer(&messageReply);
	CloseSocketWrapper(&multicastSocketWrapper);
	CloseSocketWrapper(&dataPayload);
	return integerOfNewSocketWrapper;
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

int InitializeMulticastSocketWrapper(SocketWrapper *sw, int flags)
{

	PopulateSocketWrapper(sw, 
						  "Multicast", 
						  DEFAULT_ESPA_MULTICAST_IP,
						  DEFAULT_ESPA_MULTICAST_PORT,
						  AF_INET,
						  SOCK_DGRAM,
						  flags);
	InitializeSocketWrapper(sw);

	return 0;
}

