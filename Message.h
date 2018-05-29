/**
@file
@brief The header for sending messages using the Mailman utility

When sending a message, the .from field is automatically populated
based on the address name specified as part of the CreateAddress()
call. 

By convention, the receiving process is responsible for knowing
how to parse the id fields of a message it receives on an address

The way to declare a message is as follows:

@code
    char buffer[200];
    strcpy(buffer, "Hello, Bob!");

    Message m = {0};
    strcpy(m.from, "Alice");
    m.id 	= 0;
    m.dlen 	= strlen(buffer);
    m.data 	= buffer;
@endcode

@authors David Brandman and Benjamin Shanahan

*/
#pragma once

#include <inttypes.h>    // For uint#_t compatibility


#define PROCESS_MAX_CHARS 32 
#define DEFAULT_MESSAGE_BUFFER_SIZE 1500

typedef struct
{
	char 		from[PROCESS_MAX_CHARS];
	uint8_t     id;
	uint32_t 	dlen;
	void* 		data; 

} Message;


/**
@brief Print the message header and assume the data is a string
*/
void PrintMessage(Message *m);

/**
* @brief Compare if two messages are equal in content
*/
int AreMessagesEqual(Message *m1, Message *m2);

/**
@brief Populate a Message with some values!
*/
Message CreateMessage(char *name, uint8_t id, void *data, uint32_t dlen);

/**
@brief Populate a Message with just a name, otherwise it's empty.
*/
Message CreateEmptyMessage(char *name);

/**
@brief In case you'd like to create a message buffer, this is a simple command!
*/
Message CreateMessageBuffer(int bufferLength);

/**
@brief Once you've created a message buffer, it's important to clear the memory!
*/
void DestroyMessageBuffer(Message *m);
