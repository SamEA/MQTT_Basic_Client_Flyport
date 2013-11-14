/* **************************************************************************
 *  FileName:        mqtt.h
 *  Module:          FlyPort WI-FI
 *
 *  Author               Rev.    Date              Comment
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *  SamEA                1.0    08/01/2013		      needs to be tested
 *  
*             MQTT - A simple client for MQTT and Flyport 
 *
 *   This is a porting for Flyport of Nicholas O'Leary original work PubSubClient.cpp 
 *  					      more at http://knolleary.net and mqtt.org
 *  					                                 
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */


#ifndef mqtt_h
#define mqtt_h

#include "TCPlib.h"

// MQTT_MAX_PACKET_SIZE : Maximum packet size
#define MQTT_MAX_PACKET_SIZE 128

// MQTT_KEEPALIVE : keepAlive interval in Seconds
#define MQTT_KEEPALIVE 15

#define MQTTPROTOCOLVERSION 3
#define MQTTCONNECT     1 << 4  // Client request to connect to Server
#define MQTTCONNACK     2 << 4  // Connect Acknowledgment
#define MQTTPUBLISH     3 << 4  // Publish message
#define MQTTPUBACK      4 << 4  // Publish Acknowledgment
#define MQTTPUBREC      5 << 4  // Publish Received (assured delivery part 1)
#define MQTTPUBREL      6 << 4  // Publish Release (assured delivery part 2)
#define MQTTPUBCOMP     7 << 4  // Publish Complete (assured delivery part 3)
#define MQTTSUBSCRIBE   8 << 4  // Client Subscribe request
#define MQTTSUBACK      9 << 4  // Subscribe Acknowledgment
#define MQTTUNSUBSCRIBE 10 << 4 // Client Unsubscribe request
#define MQTTUNSUBACK    11 << 4 // Unsubscribe Acknowledgment
#define MQTTPINGREQ     12 << 4 // PING Request
#define MQTTPINGRESP    13 << 4 // PING Response
#define MQTTDISCONNECT  14 << 4 // Client is Disconnecting
#define MQTTReserved    15 << 4 // Reserved

#define MQTTQOS0        (0 << 1)
#define MQTTQOS1        (1 << 1)
#define MQTTQOS2        (2 << 1)

   UINT8 buffer[MQTT_MAX_PACKET_SIZE];
   UINT16 nextMsgId;
   unsigned long lastOutActivity;
   unsigned long lastInActivity;
   BOOL pingOutstanding;
   void (*callback)(char*,UINT8*,unsigned int);
   UINT16 mqttReadPacket(UINT8*);
   UINT8 mqttReadByte();
   BOOL mqttWrite(UINT8 header, UINT8* buf, UINT16 length);
   UINT16 mqttWriteString(char* string, UINT8* buf, UINT16 pos);
   UINT8 *ip;
   char* domain;
   UINT16 port;
   
   // Update these with values suitable for your network.
   BYTE mac[6]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
   BYTE defaultIP[4] = { 172, 16, 0, 2 };
   BYTE ip[4]     = { 172, 16, 0, 100 };



   
   mqtt();
   BOOL mqttConnect(char *, char *, char *, char *, UINT8, UINT8, char*);
   void mqttDisconnect();
   BOOL mqttPublish_P(char *, UINT8 PROGMEM *, unsigned int, boolean);
   BOOL mqttSubscribe(char *);
   BOOL mqttUnsubscribe(char *);
   BOOL mqttLoop();


#endif