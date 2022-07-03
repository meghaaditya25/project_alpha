
#include "StackTrace.h"
#include "MQTTPacket.h"
#include <string.h>

#define min(a, b) ((a < b) ? a : b)


/**
  * Validates MQTT protocol name and version combinations
  */
int MQTTPacket_checkVersion(MQTTString* protocol, int version)
{
	int rc = 0;

	if (version == 3 && memcmp(protocol->lenstring.data, "MQIsdp",
			min(6, protocol->lenstring.len)) == 0)
		rc = 1;
	else if (version == 4 && memcmp(protocol->lenstring.data, "MQTT",
			min(4, protocol->lenstring.len)) == 0)
		rc = 1;
	return rc;
}


/**
  * Deserializes the supplied (wire) buffer into connect data structure
  */
int MQTTDeserialize_connect(MQTTPacket_connectData* data, unsigned char* buf, int len)
{
	MQTTHeader header = {0};
	MQTTConnectFlags flags = {0};
	unsigned char* curdata = buf;
	unsigned char* enddata = &buf[len];
	int rc = 0;
	MQTTString Protocol;
	int version;
	int mylen = 0;

	FUNC_ENTRY;
	header.byte = readChar(&curdata);
	if (header.bits.type != CONNECT)
		goto exit;

	curdata += MQTTPacket_decodeBuf(curdata, &mylen);

	if (!readMQTTLenString(&Protocol, &curdata, enddata) ||
		enddata - curdata < 0)
		goto exit;

	version = (int)readChar(&curdata); /* Protocol version */

	if (MQTTPacket_checkVersion(&Protocol, version))
	{
		flags.all = readChar(&curdata);
		data->cleansession = flags.bits.cleansession;
		data->keepAliveInterval = readInt(&curdata);
		if (!readMQTTLenString(&data->clientID, &curdata, enddata))
			goto exit;
		data->willFlag = flags.bits.will;
		if (flags.bits.will)
		{
			data->will.qos = flags.bits.willQoS;
			data->will.retained = flags.bits.willRetain;
			if (!readMQTTLenString(&data->will.topicName, &curdata, enddata) ||
				  !readMQTTLenString(&data->will.message, &curdata, enddata))
				goto exit;
		}
		if (flags.bits.username)
		{
			if (enddata - curdata < 3 || !readMQTTLenString(&data->username, &curdata, enddata))
				goto exit;
			if (flags.bits.password &&
				(enddata - curdata < 3 || !readMQTTLenString(&data->password, &curdata, enddata)))
				goto exit;
		}
		else if (flags.bits.password)
			goto exit;
		rc = 1;
	}
exit:
	FUNC_EXIT_RC(rc);
	return rc;
}


/**
  * Serializes the connack packet into the supplied buffer.
  */
int MQTTSerialize_connack(unsigned char* buf, int buflen, unsigned char connack_rc, unsigned char sessionPresent)
{
	MQTTHeader header = {0};
	int rc = 0;
	unsigned char *ptr = buf;
	MQTTConnackFlags flags = {0};

	FUNC_ENTRY;
	if (buflen < 2)
	{
		rc = MQTTPACKET_BUFFER_TOO_SHORT;
		goto exit;
	}
	header.byte = 0;
	header.bits.type = CONNACK;
	writeChar(&ptr, header.byte);

	ptr += MQTTPacket_encode(ptr, 2);

	flags.all = 0;
	flags.bits.sessionpresent = sessionPresent;
	writeChar(&ptr, flags.all); 
	writeChar(&ptr, connack_rc);

	rc = ptr - buf;
exit:
	FUNC_EXIT_RC(rc);
	return rc;
}

