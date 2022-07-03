
#include "MQTTPacket.h"
#include "StackTrace.h"

#include <string.h>


int MQTTDeserialize_subscribe(unsigned char* dup, unsigned short* packetid, int maxcount, int* count, MQTTString topicFilters[],
	int requestedQoSs[], unsigned char* buf, int buflen)
{
	MQTTHeader header = {0};
	unsigned char* curdata = buf;
	unsigned char* enddata = NULL;
	int rc = -1;
	int mylen = 0;

	FUNC_ENTRY;
	header.byte = readChar(&curdata);
	if (header.bits.type != SUBSCRIBE)
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
		if (curdata >= enddata) /* do we have enough data to read the req_qos version byte? */
			goto exit;
		requestedQoSs[*count] = readChar(&curdata);
		(*count)++;
	}

	rc = 1;
exit:
	FUNC_EXIT_RC(rc);
	return rc;
}


/**
  * Serializes the supplied suback data into the supplied buffer, ready for sending
  */
int MQTTSerialize_suback(unsigned char* buf, int buflen, unsigned short packetid, int count, int* grantedQoSs)
{
	MQTTHeader header = {0};
	int rc = -1;
	unsigned char *ptr = buf;
	int i;

	FUNC_ENTRY;
	if (buflen < 2 + count)
	{
		rc = MQTTPACKET_BUFFER_TOO_SHORT;
		goto exit;
	}
	header.byte = 0;
	header.bits.type = SUBACK;
	writeChar(&ptr, header.byte); /* write header */

	ptr += MQTTPacket_encode(ptr, 2 + count); /* write remaining length */

	writeInt(&ptr, packetid);

	for (i = 0; i < count; ++i)
		writeChar(&ptr, grantedQoSs[i]);

	rc = ptr - buf;
exit:
	FUNC_EXIT_RC(rc);
	return rc;
}


