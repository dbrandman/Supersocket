#include "Supersocket.h"
#include "ManageHeapMemory.h"
#include "SupersocketListener.h"
#include <unistd.h> // For close

#include <errno.h>

/**
Add code to ensure name is not too long

I like the bitmask idea, it makes it much easier to wrok with.
*/



static void swap(int *a, int *b);


/**
 This will add a UDP address for AF_INET and AF_UNIX
*/
int InitializeSupersocket(Supersocket *s, char *name, char *ip, int port)
{
	strcpy(s->name, name);
	s->nSockets 			= 0;
	s->nBoundSockets 		= 0;
	s->nConnectedSockets 	= 0;

	if(pthread_mutex_init(&s->lock, NULL) < 0)
	{
		DisplayError("Could not initialize mutex lock: %s", strerror(errno));
		return -1;
	}

	if(AddSocket(s, name, ip, port, AF_INET, SOCK_DGRAM, BIND) < 0)
	{
		DisplayError("Could not initialize AF_INET SOCK_DGRAM: %s", strerror(errno));
		return -1;		
	}
	if(AddSocket(s, name, ip, port, AF_UNIX, SOCK_DGRAM, BIND) < 0)
	{
		DisplayError("Could not initialize AF_UNIX SOCK_DGRAM: %s", strerror(errno));
		return -1;		
	}

	return 0;
}

int CloseSupersocket(Supersocket *s)
{
	pthread_mutex_lock(&s->lock);
	for (int i = 0; i < s->nSockets; i++)
		close(s->socketWrapper[i].socket);

	pthread_mutex_unlock(&s->lock);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

int AddSocket(Supersocket *s, char *name, char *ip, int port, int domain, int type, int flags)
{
	SocketWrapper sw = {0};
	PopulateSocketWrapper(&sw, name, ip, port, domain, type, flags);
	return AddSocketWrapper(s, &sw);
}

/*
 * This returns the integer corresponding to the address for the new entry
 */
int AddSocketWrapper(Supersocket *s, SocketWrapper *sw)
{
	pthread_mutex_lock(&s->lock);
	// We are going to be adding the entry at address n in the master socket list.
	int n = s->nSockets;
	if (InitializeSocketWrapper(sw) < 0)
	{
		DisplayError("[%s] Could not initialize %s", s->name, sw->name);
		return -1;
	}
	else
	{
		if(ParseFlags(sw->flags, BIND))
		{
			int m = s->nBoundSockets;
			s->boundSocketsList         	= ManageHeapMemory(s->boundSocketsList, m, sizeof(int));
			s->boundSocketsList[m] 			= n;		
			
			s->boundSocketsStruct         	= ManageHeapMemory(s->boundSocketsStruct, m, sizeof(struct pollfd));
			s->boundSocketsStruct[m].fd 	= sw->socket;
			s->boundSocketsStruct[m].events = POLLIN;

			s->nBoundSockets++;
		}
		if(ParseFlags(sw->flags, CONNECT))
		{
			int m = s->nConnectedSockets;
			s->connectedSocketsList         = ManageHeapMemory(s->connectedSocketsList, m, sizeof(int));
			s->connectedSocketsList[m] 		= n;
			s->nConnectedSockets++;
		}
	}
	s->socketWrapper = ManageHeapMemory(s->socketWrapper, n, sizeof(SocketWrapper));
	memcpy(&s->socketWrapper[n], sw, sizeof(SocketWrapper)); 

	s->nSockets++;
	pthread_mutex_unlock(&s->lock);
	return n;	
}


/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

int SendData(Supersocket *s, int target, void *data, int dlen, MessagingOptions *options)
{
	// Perform some error checking to ensure that the target is valid
	if (target > s->nSockets)
	{
		DisplayError("SendData: Target number exceeds number of sockets!");
		return -1;
	}

	// Send data only to the socketWrapper array entry that is requested
	return SendDataToSocketWrapper(&s->socketWrapper[target], data, dlen, options);
}

int SendDataToAll(Supersocket *s, void *data, int dlen, MessagingOptions *options)
{
	for(int i = 0; i < s->nConnectedSockets; i++)
		SendDataToSocketWrapper(&s->socketWrapper[s->connectedSocketsList[i]], data, dlen, options);
	
	return 0;
}

int SendMessage(Supersocket *s, int target, Message *m)
{
	// Perform some error checking to ensure that the target is valid
	if (target > s->nSockets)
	{
		DisplayError("SendMessage: Target number exceeds number of sockets!");
		return -1;
	}
	
	// Send message only to the socketWrapper array that is requested	
	return SendMessageToSocketWrapper(&s->socketWrapper[target], m);	
}


int SendMessageToAll(Supersocket *s, Message *m)
{
	for(int i = 0; i < s->nConnectedSockets; i++)
		SendMessageToSocketWrapper(&s->socketWrapper[s->connectedSocketsList[i]], m);	
	
	return 0;
}


int PollSockets(Supersocket *s, int milliseconds)
{
	int val = poll(s->boundSocketsStruct, s->nBoundSockets, milliseconds);
	if (val < 0)
	{
		DisplayError("Could not poll bound sockets: %s", strerror(errno));
		return -1;	
	}
	return val;
}

int ReceiveSupersocket(Supersocket *s, int receiveMessageFlag, Message *m, void *data, int dlen, MessagingOptions *options)
{
	int output = 0;
	for(int i = 0; i < s->nBoundSockets; i++)
		if(s->boundSocketsStruct[i].revents == POLLIN)
		{
			if(receiveMessageFlag == 1)
				output = ReceiveMessageFromSocketWrapper(&s->socketWrapper[s->boundSocketsList[i]], m);
			else
				output = ReceiveDataFromSocketWrapper(&s->socketWrapper[s->boundSocketsList[i]], data, dlen, options);

			if(output >= 0)
			{
				swap(&s->boundSocketsList[i], &s->boundSocketsList[s->nBoundSockets-1]);
				swap(&s->boundSocketsStruct[i].fd, &s->boundSocketsStruct[s->nBoundSockets-1].fd);
			}
			return output;
		}	

	return -1;
}

int ReceiveData(Supersocket *s, void *data, int dlen, MessagingOptions *options)
{
	if(PollSockets(s, -1) < 0)
	{
		DisplayError("Unable to poll socket: %s",  strerror(errno));		
		return -1;
	}

	return ReceiveSupersocket(s, 0, NULL, data, dlen, options);
}

int ReceiveMessage(Supersocket *s, Message *m)
{
	if(PollSockets(s, -1) < 0)
	{
		DisplayError("Unable to poll socket: %s", strerror(errno));		
		return -1;
	}

	return ReceiveSupersocket(s, 1, m, NULL, 0, NULL);
}

// int ReceiveMessage(Supersocket *s, Message *m)
// {
// 	return PollAndReceiveSupersocket(s, 1, m, NULL, 0, NULL);
// }


/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////


/**

*/
void PrintSupersocket(Supersocket *s)
{
	Display("Supersocket name: %s", s->name);
	Display("SocketWrapper objects: %d", s->nSockets);
	Display("Number of bound sockets: %d", s->nBoundSockets);
	Display("Number of connected sockets: %d", s->nConnectedSockets);
	for(int i = 0 ; i < s->nSockets; i++)
		PrintSocketWrapper(&s->socketWrapper[i]);
}

void VerboseSupersocket(Supersocket *s)
{
	SetAutoNewline(DISABLE);

	Display("Socket List: "); 
	SetShowTrace(DISABLE);
	for(int i = 0; i < s->nSockets; i++)
		Display("%d ", s->socketWrapper[i].socket);
	Display("\n");
	SetShowTrace(ENABLE);

	Display("Bound Socket Indexes: ");
	SetShowTrace(DISABLE);
	for(int i = 0; i < s->nBoundSockets; i++)
		Display("%d ", s->boundSocketsList[i]);
	Display("\n");	
	SetShowTrace(ENABLE);

	Display("Connected Socket Indexes: ");
	SetShowTrace(DISABLE);
	for(int i = 0; i < s->nConnectedSockets; i++)
		Display("%d ", s->connectedSocketsList[i]);
	Display("\n");	
	SetAutoNewline(ENABLE);
	SetShowTrace(ENABLE);
}

static void swap(int *a, int *b)
{
	int temp = *b;
	*b = *a;
	*a = temp;
}

/*
	// pthread_mutex_lock(&s->lock);
	// int n = s->nSockets;

	// s->socketWrapper = ManageHeapMemory(s->socketWrapper, n, sizeof(SocketWrapper));
	SocketWrapper sw = {0};
	PopulateSocketWrapper(&sw, name, ip, port, domain, type, flags);
	AddSocketWrapper(s, &sw);

	return 0;

	// if (InitializeSocketWrapper(&s->socketWrapper[n]) < 0)
	// {
	// 	DisplayError("Could not initialize %s", name);
	// 	return -1;
	// }
	// else
	// {
	// 	if(ParseFlags(flags, BIND))
	// 	{
	// 		int m = s->nBoundSockets;

	// 		s->boundSocketsList         	= ManageHeapMemory(s->boundSocketsList, m, sizeof(int));
	// 		s->boundSocketsList[m] 			= n;		

	// 		s->boundSocketsStruct         	= ManageHeapMemory(s->boundSocketsStruct, m, sizeof(struct pollfd));
	// 		s->boundSocketsStruct[m].fd 	= s->socketWrapper[n].socket;
	// 		s->boundSocketsStruct[m].events = POLLIN;
	// 		s->nBoundSockets++;
	// 	}
	// 	if(ParseFlags(flags, CONNECT))
	// 	{
	// 		int m = s->nConnectedSockets;
	// 		s->connectedSocketsList         = ManageHeapMemory(s->connectedSocketsList, m, sizeof(int));
	// 		s->connectedSocketsList[m] 		= n;
	// 		s->nConnectedSockets++;
	// 	}
	// }
	// s->nSockets++;

	// pthread_mutex_unlock(&s->lock);
	// return 0;

// static int PollAndReceiveSupersocket(Supersocket *s, int receiveMessageFlag, Message *m, void *data, int dlen, MessagingOptions *options)
// {
// 	int n = PollSockets(s, -1);
// 	if (n < 0)
// 		return -1;

// 	int output = 0;
// 	for(int i = 0; i < s->nBoundSockets; i++)
// 		if(s->boundSocketsStruct[i].revents == POLLIN)
// 		{
// 			if(receiveMessageFlag == 1)
// 				output = ReceiveMessageFromSocketWrapper(&s->socketWrapper[s->boundSocketsList[i]], m);
// 			else
// 				output = ReceiveDataFromSocketWrapper(&s->socketWrapper[s->boundSocketsList[i]], data, dlen, options);

// 			if(output >= 0)
// 			{
// 				swap(&s->boundSocketsList[i], &s->boundSocketsList[s->nBoundSockets-1]);
// 				swap(&s->boundSocketsStruct[i].fd, &s->boundSocketsStruct[s->nBoundSockets-1].fd);
// 			}
// 			return output;
// 		}	
// }
*/
