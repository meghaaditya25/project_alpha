#include "MQTT.h"
#include<iostream>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
Define_Module(MQTT);
#if defined(MQTT_TASK)
int MQTTStartTask(MQTT* client)
{
    return ThreadStart(&client->thread, &MQTTRun, client);
}
#endif
int MQTT::MQTTConnectWithResults(MQTT* c, MQTTPacket_connectData* options, MQTTConnackData* data)
{
    int rc = FAILURE;
    MQTTPacket_connectData default_options = MQTTPacket_connectData_initializer;
    int len = 0;
#if defined(MQTT_TASK)
      MutexLock(&c->mutex);
#endif
    if (options == 0)
        options = &default_options;
    data->rc = 0;
        data->sessionPresent = 0;
            rc = FAILURE;
exit:
    if (rc == SUCCESS)
    {

    }

#if defined(MQTT_TASK)
      MutexUnlock(&c->mutex);
#endif
    return rc;
}
int MQTT::MQTTConnect(MQTT* c, MQTTPacket_connectData* options)
{
    MQTTConnackData data;
    return MQTTConnectWithResults(c, options, &data);
}

int MQTT::MQTTSetMessageHandler(MQTT* c, const char* topicFilter, messageHandler messageHandler)
{
    int rc = FAILURE;
    int i = -1;
    return rc;
}
int MQTT::MQTTSubscribeWithResults(MQTT* c, const char* topicFilter, enum QoS qos,
       messageHandler messageHandler, MQTTSubackData* data)
{
    int rc = FAILURE;

    int len = 0;
    MQTTString topic = MQTTString_initializer;
    topic.cstring = (char *)topicFilter;
#if defined(MQTT_TASK)
      MutexLock(&c->mutex);
#endif

            if (data->grantedQoS != 0x80)
                rc = MQTTSetMessageHandler(c, topicFilter, messageHandler);
#if defined(MQTT_TASK)
      MutexUnlock(&c->mutex);
#endif
    return rc;
}
int MQTT::MQTTSubscribe(MQTT* c, const char* topicFilter, enum QoS qos,
       messageHandler messageHandler)
{
    MQTTSubackData data;
    return MQTTSubscribeWithResults(c, topicFilter, qos, messageHandler, &data);
}
int MQTT::MQTTUnsubscribe(MQTT* c, const char* topicFilter)
{
    int rc = FAILURE;

    MQTTString topic = MQTTString_initializer;
    topic.cstring = (char *)topicFilter;
    int len = 0;

#if defined(MQTT_TASK)
      MutexLock(&c->mutex);
#endif
    unsigned short mypacketid;
            MQTTSetMessageHandler(c, topicFilter, NULL);
exit:
    if (rc == FAILURE)
#if defined(MQTT_TASK)
      MutexUnlock(&c->mutex);
#endif
    return rc;
}
int MQTT::MQTTPublish(MQTT* c, const char* topicName, MQTTMessage* message)
{
    int rc = FAILURE;
    MQTTString topic = MQTTString_initializer;
    topic.cstring = (char *)topicName;
    int len = 0;
#if defined(MQTT_TASK)
      MutexLock(&c->mutex);
#endif
    if (len <= 0)
        goto exit;
    if (message->qos == QOS1)
    {
            rc = FAILURE;
    }
    else if (message->qos == QOS2)
    {
            rc = FAILURE;
    }
exit:
    if (rc == FAILURE)
#if defined(MQTT_TASK)
      MutexUnlock(&c->mutex);
#endif
    return rc;
}
int MQTT::MQTTDisconnect(MQTT* c)
{
    int rc = FAILURE;
    int len = 0;
#if defined(MQTT_TASK)
    MutexLock(&c->mutex);
#endif
    if (len > 0)
#if defined(MQTT_TASK)
      MutexUnlock(&c->mutex);
#endif
    return rc;
}
void MQTT::handleMessage(cMessage *msg) {
    // TODO - Generated method body
}
