
#include "MQTTPacket.h"
#include "StackTrace.h"

#include <string.h>

/**
  * Determines the length of the MQTT unsubscribe packet that would be produced using the supplied parameters
  */
int MQTTSerialize_unsubscribeLength(int count, MQTTString topicFilters[])
{
	int i;
	int len = 2;

	for (i = 0; i < count; ++i)
		len += 2 + MQTTstrlen(topicFilters[i]); /* length + topic*/
	return len;
}


/**
  * Serializes the supplied unsubscribe data into the supplied buffer, ready for sending
  */
int MQTTSerialize_unsubscribe(unsigned char* buf, int buflen, unsigned char dup, unsigned short packetid,
		int count, MQTTString topicFilters[])
{
	unsigned char *ptr = buf;
	MQTTHeader header = {0};
	int rem_len = 0;
	int rc = -1;
	int i = 0;

	FUNC_ENTRY;
	if (MQTTPacket_len(rem_len = MQTTSerialize_unsubscribeLength(count, topicFilters)) > buflen)
	{
		rc = MQTTPACKET_BUFFER_TOO_SHORT;
		goto exit;
	}

	header.byte = 0;
	header.bits.type = UNSUBSCRIBE;
	header.bits.dup = dup;
	header.bits.qos = 1;
	writeChar(&ptr, header.byte); /* write header */

	ptr += MQTTPacket_encode(ptr, rem_len); /* write remaining length */;

	writeInt(&ptr, packetid);

	for (i = 0; i < count; ++i)
		writeMQTTString(&ptr, topicFilters[i]);

	rc = ptr - buf;
exit:
	FUNC_EXIT_RC(rc);
	return rc;
}


/**
  * Deserializes the supplied (wire) buffer into unsuback data
  */
int MQTTDeserialize_unsuback(unsigned short* packetid, unsigned char* buf, int buflen)
{
	unsigned char type = 0;
	unsigned char dup = 0;
	int rc = 0;

	FUNC_ENTRY;
	rc = MQTTDeserialize_ack(&type, &dup, packetid, buf, buflen);
	if (type == UNSUBACK)
		rc = 1;
	FUNC_EXIT_RC(rc);
	return rc;
}


