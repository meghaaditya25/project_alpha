
#include "StackTrace.h"
#include "MQTTPacket.h"

#include <string.h>

/**
 * Encodes the message length according to the MQTT algorithm
 */
int MQTTPacket_encode(unsigned char* buf, int length)
{
	int rc = 0;

	FUNC_ENTRY;
	do
	{
		char d = length % 128;
		length /= 128;
		/* if there are more digits to encode, set the top bit of this digit */
		if (length > 0)
			d |= 0x80;
		buf[rc++] = d;
	} while (length > 0);
	FUNC_EXIT_RC(rc);
	return rc;
}


/**
 * Decodes the message length according to the MQTT algorithm
 */
int MQTTPacket_decode(int (*getcharfn)(unsigned char*, int), int* value)
{
	unsigned char c;
	int multiplier = 1;
	int len = 0;
#define MAX_NO_OF_REMAINING_LENGTH_BYTES 4

	FUNC_ENTRY;
	*value = 0;
	do
	{
		int rc = MQTTPACKET_READ_ERROR;

		if (++len > MAX_NO_OF_REMAINING_LENGTH_BYTES)
		{
			rc = MQTTPACKET_READ_ERROR;	/* bad data */
			goto exit;
		}
		rc = (*getcharfn)(&c, 1);
		if (rc != 1)
			goto exit;
		*value += (c & 127) * multiplier;
		multiplier *= 128;
	} while ((c & 128) != 0);
exit:
	FUNC_EXIT_RC(len);
	return len;
}


int MQTTPacket_len(int rem_len)
{
	rem_len += 1; /* header byte */

	/* now remaining_length field */
	if (rem_len < 128)
		rem_len += 1;
	else if (rem_len < 16384)
		rem_len += 2;
	else if (rem_len < 2097151)
		rem_len += 3;
	else
		rem_len += 4;
	return rem_len;
}


static unsigned char* bufptr;

int bufchar(unsigned char* c, int count)
{
	int i;

	for (i = 0; i < count; ++i)
		*c = *bufptr++;
	return count;
}


int MQTTPacket_decodeBuf(unsigned char* buf, int* value)
{
	bufptr = buf;
	return MQTTPacket_decode(bufchar, value);
}


int readInt(unsigned char** pptr)
{
	unsigned char* ptr = *pptr;
	int len = 256*(*ptr) + (*(ptr+1));
	*pptr += 2;
	return len;
}


char readChar(unsigned char** pptr)
{
	char c = **pptr;
	(*pptr)++;
	return c;
}

void writeChar(unsigned char** pptr, char c)
{
	**pptr = c;
	(*pptr)++;
}


void writeInt(unsigned char** pptr, int anInt)
{
	**pptr = (unsigned char)(anInt / 256);
	(*pptr)++;
	**pptr = (unsigned char)(anInt % 256);
	(*pptr)++;
}


void writeCString(unsigned char** pptr, const char* string)
{
	int len = strlen(string);
	writeInt(pptr, len);
	memcpy(*pptr, string, len);
	*pptr += len;
}


int getLenStringLen(char* ptr)
{
	int len = 256*((unsigned char)(*ptr)) + (unsigned char)(*(ptr+1));
	return len;
}


void writeMQTTString(unsigned char** pptr, MQTTString mqttstring)
{
	if (mqttstring.lenstring.len > 0)
	{
		writeInt(pptr, mqttstring.lenstring.len);
		memcpy(*pptr, mqttstring.lenstring.data, mqttstring.lenstring.len);
		*pptr += mqttstring.lenstring.len;
	}
	else if (mqttstring.cstring)
		writeCString(pptr, mqttstring.cstring);
	else
		writeInt(pptr, 0);
}


int readMQTTLenString(MQTTString* mqttstring, unsigned char** pptr, unsigned char* enddata)
{
	int rc = 0;

	FUNC_ENTRY;
	/* the first two bytes are the length of the string */
	if (enddata - (*pptr) > 1) /* enough length to read the integer? */
	{
		mqttstring->lenstring.len = readInt(pptr); /* increments pptr to point past length */
		if (&(*pptr)[mqttstring->lenstring.len] <= enddata)
		{
			mqttstring->lenstring.data = (char*)*pptr;
			*pptr += mqttstring->lenstring.len;
			rc = 1;
		}
	}
	mqttstring->cstring = NULL;
	FUNC_EXIT_RC(rc);
	return rc;
}


int MQTTstrlen(MQTTString mqttstring)
{
	int rc = 0;

	if (mqttstring.cstring)
		rc = strlen(mqttstring.cstring);
	else
		rc = mqttstring.lenstring.len;
	return rc;
}


int MQTTPacket_equals(MQTTString* a, char* bptr)
{
	int alen = 0,
		blen = 0;
	char *aptr;
	
	if (a->cstring)
	{
		aptr = a->cstring;
		alen = strlen(a->cstring);
	}
	else
	{
		aptr = a->lenstring.data;
		alen = a->lenstring.len;
	}
	blen = strlen(bptr);
	
	return (alen == blen) && (strncmp(aptr, bptr, alen) == 0);
}


int MQTTPacket_read(unsigned char* buf, int buflen, int (*getfn)(unsigned char*, int))
{
	int rc = -1;
	MQTTHeader header = {0};
	int len = 0;
	int rem_len = 0;


	if ((*getfn)(buf, 1) != 1)
		goto exit;

	len = 1;

	MQTTPacket_decode(getfn, &rem_len);
	len += MQTTPacket_encode(buf + 1, rem_len); /* put the original remaining length back into the buffer */


	if((rem_len + len) > buflen)
		goto exit;
	if (rem_len && ((*getfn)(buf + len, rem_len) != rem_len))
		goto exit;

	header.byte = buf[0];
	rc = header.bits.type;
exit:
	return rc;
}

static int MQTTPacket_decodenb(MQTTTransport *trp)
{
	unsigned char c;
	int rc = MQTTPACKET_READ_ERROR;

	FUNC_ENTRY;
	if(trp->len == 0){		/* initialize on first call */
		trp->multiplier = 1;
		trp->rem_len = 0;
	}
	do {
		int frc;
		if (trp->len >= MAX_NO_OF_REMAINING_LENGTH_BYTES)
			goto exit;
		if ((frc=(*trp->getfn)(trp->sck, &c, 1)) == -1)
			goto exit;
		if (frc == 0){
			rc = 0;
			goto exit;
		}
		++(trp->len);
		trp->rem_len += (c & 127) * trp->multiplier;
		trp->multiplier *= 128;
	} while ((c & 128) != 0);
	rc = trp->len;
exit:
	FUNC_EXIT_RC(rc);
	return rc;
}

int MQTTPacket_readnb(unsigned char* buf, int buflen, MQTTTransport *trp)
{
	int rc = -1, frc;
	MQTTHeader header = {0};

	switch(trp->state){
	default:
		trp->state = 0;
		/*FALLTHROUGH*/
	case 0:

		if ((frc=(*trp->getfn)(trp->sck, buf, 1)) == -1)
			goto exit;
		if (frc == 0)
			return 0;
		trp->len = 0;
		++trp->state;

	case 1:
		if((frc=MQTTPacket_decodenb(trp)) == MQTTPACKET_READ_ERROR)
			goto exit;
		if(frc == 0)
			return 0;
		trp->len = 1 + MQTTPacket_encode(buf + 1, trp->rem_len);
		if((trp->rem_len + trp->len) > buflen)
			goto exit;
		++trp->state;

	case 2:
		if(trp->rem_len){

			if ((frc=(*trp->getfn)(trp->sck, buf + trp->len, trp->rem_len)) == -1)
				goto exit;
			if (frc == 0)
				return 0;
			trp->rem_len -= frc;
			trp->len += frc;
			if(trp->rem_len)
				return 0;
		}
		header.byte = buf[0];
		rc = header.bits.type;
		break;
	}

exit:
	trp->state = 0;
	return rc;
}

