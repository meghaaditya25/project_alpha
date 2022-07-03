
#include "MQTTPacket.h"
#include "StackTrace.h"

#include <string.h>


int MQTTDeserialize_unsubscribe(unsigned char* dup, unsigned short* packetid, int maxcount, int* count, MQTTString topicFilters[],
		unsigned char* buf, int len)
{
	MQTTHeader header = {0};
	unsigned char* curdata = buf;
	unsigned char* enddata = NULL;
	int rc = 0;
	int mylen = 0;

	FUNC_ENTRY;
	header.byte = readChar(&curdata);
	if (header.bits.type != UNSUBSCRIBE)
		goto exit;
	*dup = header.bits.dup;

	curdata += (rc = MQTTPacket_decodeBuf(curdata, &mylen)); /* read remaining length */
	enddata = curdata + mylen;

	*packetid = readInt(&curdata);

	*count = 0;
	while (curdata < enddata)
	{
		if (!readMQTTLenString(&topicFilters[*count], &curdata, enddata))
			goto exit;
		(*count)++;
	}

	rc = 1;
exit:
	FUNC_EXIT_RC(rc);
	return rc;
}


/**
  * Serializes the supplied unsuback data into the supplied buffer, ready for sending
  */
int MQTTSerialize_unsuback(unsigned char* buf, int buflen, unsigned short packetid)
{
	MQTTHeader header = {0};
	int rc = 0;
	unsigned char *ptr = buf;

	FUNC_ENTRY;
	if (buflen < 2)
	{
		rc = MQTTPACKET_BUFFER_TOO_SHORT;
		goto exit;
	}
	header.byte = 0;
	header.bits.type = UNSUBACK;
	writeChar(&ptr, header.byte); /* write header */

	ptr += MQTTPacket_encode(ptr, 2); /* write remaining length */

	writeInt(&ptr, packetid);

	rc = ptr - buf;
exit:
	FUNC_EXIT_RC(rc);
	return rc;
}


