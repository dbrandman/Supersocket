
#include "Display.h"
#include "Message.h"
#include <stdlib.h> // For malloc()
#include <string.h>

Message CreateMessage(char *name, uint8_t id, void *data, uint32_t dlen)
{
	Message m = {0};
	strcpy(m.from, name);
	m.id 		= id;
	m.dlen 		= dlen;
	m.data 		= data;

	return m;
}

Message CreateEmptyMessage(char *name)
{
	Message m = {0};
	strcpy(m.from, name);
	
	return m;
}

Message CreateMessageBuffer(int bufferLength)
{
	Message m 	= {0};
	m.data 		= malloc(bufferLength);
	m.dlen 		= bufferLength;

	return m;
}

void DestroyMessageBuffer(Message *m)
{
	free(m->data);
}

void PrintMessage(Message *m)
{
	Display("From    : %s", m->from);
	Display("ID      : %u", m->id);
	Display("Length  : %u", m->dlen);
	Display("Data    : %s", m->data);
}

/**
@brief compare if two message are the same. Since data contains a pointer
the most verbose way to do this is to compare each element seperately
*/
int AreMessagesEqual(Message *m1, Message *m2)
{

	if(strcmp(m1->from, m2->from) != 0)
		return 0;

	if (m1->id != m2->id)
		return 0;

	if (m1->dlen !=m2->dlen)
		return 0;

	char *m1DataPointer = m1->data;
	char *m2DataPointer = m2->data; 

	for(int i = 0; i < m1->dlen; i++)
		if(*m1DataPointer++ != *m2DataPointer++)
			return 0;



	return 1;
}
