
#ifndef MQTTCONNECT_H_
#define MQTTCONNECT_H_

enum connack_return_codes
{
    MQTT_CONNECTION_ACCEPTED = 0,
    MQTT_UNNACCEPTABLE_PROTOCOL = 1,
    MQTT_CLIENTID_REJECTED = 2,
    MQTT_SERVER_UNAVAILABLE = 3,
    MQTT_BAD_USERNAME_OR_PASSWORD = 4,
    MQTT_NOT_AUTHORIZED = 5,
};

#if !defined(DLLImport)
  #define DLLImport
#endif
#if !defined(DLLExport)
  #define DLLExport
#endif


typedef union
{
	unsigned char all;
#if defined(REVERSED)
	struct
	{
		unsigned int username : 1;
		unsigned int password : 1;
		unsigned int willRetain : 1;
		unsigned int willQoS : 2;
		unsigned int will : 1;
		unsigned int cleansession : 1;
		unsigned int : 1;
	} bits;
#else
	struct
	{
		unsigned int : 1;
		unsigned int cleansession : 1;
		unsigned int will : 1;
		unsigned int willQoS : 2;
		unsigned int willRetain : 1;
		unsigned int password : 1;
		unsigned int username : 1;
	} bits;
#endif
} MQTTConnectFlags;



typedef struct
{
	char struct_id[4];
	int struct_version;
	MQTTString topicName;
	MQTTString message;
	unsigned char retained;
	char qos;
} MQTTPacket_willOptions;


#define MQTTPacket_willOptions_initializer { {'M', 'Q', 'T', 'W'}, 0, {NULL, {0, NULL}}, {NULL, {0, NULL}}, 0, 0 }


typedef struct
{
		char struct_id[4];
		int struct_version;
	unsigned char MQTTVersion;
	MQTTString clientID;
	unsigned short keepAliveInterval;
	unsigned char cleansession;
	unsigned char willFlag;
	MQTTPacket_willOptions will;
	MQTTString username;
	MQTTString password;
} MQTTPacket_connectData;

typedef union
{
	unsigned char all;
#if defined(REVERSED)
	struct
	{
    unsigned int reserved : 7;
		unsigned int sessionpresent : 1;
	} bits;
#else
	struct
	{
		unsigned int sessionpresent : 1;
    unsigned int reserved: 7;
	} bits;
#endif
} MQTTConnackFlags;

#define MQTTPacket_connectData_initializer { {'M', 'Q', 'T', 'C'}, 0, 4, {NULL, {0, NULL}}, 60, 1, 0, \
		MQTTPacket_willOptions_initializer, {NULL, {0, NULL}}, {NULL, {0, NULL}} }

DLLExport int MQTTSerialize_connect(unsigned char* buf, int buflen, MQTTPacket_connectData* options);
DLLExport int MQTTDeserialize_connect(MQTTPacket_connectData* data, unsigned char* buf, int len);

DLLExport int MQTTSerialize_connack(unsigned char* buf, int buflen, unsigned char connack_rc, unsigned char sessionPresent);
DLLExport int MQTTDeserialize_connack(unsigned char* sessionPresent, unsigned char* connack_rc, unsigned char* buf, int buflen);

DLLExport int MQTTSerialize_disconnect(unsigned char* buf, int buflen);
DLLExport int MQTTSerialize_pingreq(unsigned char* buf, int buflen);

#endif /* MQTTCONNECT_H_ */
