/* **************************************************************************
 *  FileName:        mqtt.c
 *  Module:          FlyPort WI-FI
 *
 *  Author               Rev.    Date              Comment
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *  SamEA                1.0    08/01/2013		      needs to be tested
 *       
 *  					  MQTT - A simple client for MQTT and Flyport 
 *
 *   This is a porting for Flyport of Nicholas O'Leary original work PubSubClient.cpp 
 *  					      more at http://knolleary.net and mqtt.org
 *
 *  					                                 
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
 
#include "mqtt.h"
static TCP_SOCKET Socket;
static int i;
static BOOL flagTCP=FALSE;
static BOOL flagTCPisCON=FALSE;


BYTE mqttConnect(char *id, char *user, char *pass, char* willTopic, uint8_t willQos, uint8_t willRetain, char* willMessage)
{  
  error = 0;
	char messageTS[nvalue*20];
	char bufferTS[20];
	flagTCP=0;
 uint8_t d[9] = {0x00,0x06,'M','Q','I','s','d','p',MQTTPROTOCOLVERSION};
  /***** Open TCP Client******/
 	while(flagTCP==FALSE)
	{
		if(z==5)
		{	
			z=0;
			flagTCP=TRUE;
			error=1;
		}
		else
		{	
			Socket=TCPClientOpen ( defaultIP, defaultPort);
			vTaskDelay(25);
			flagTCP=TCPisConn(Socket);
			flagTCPisCON=flagTCP;
			vTaskDelay(25);
			z++;
		}
	}
  
  if (!error) {
         nextMsgId = 1;
         uint8_t d[9] = {0x00,0x06,'M','Q','I','s','d','p',MQTTPROTOCOLVERSION};
         // Leave room in the buffer for header and variable length field
         uint16_t length = 5;
         unsigned int j;
         for (j = 0;j<9;j++) {
            buffer[length++] = d[j];
         }

         uint8_t v;
         if (willTopic) {
            v = 0x06|(willQos<<3)|(willRetain<<5);
         } else {
            v = 0x02;
         }

         if(user != NULL) {
            v = v|0x80;

            if(pass != NULL) {
               v = v|(0x80>>1);
            }
         }

         buffer[length++] = v;

         buffer[length++] = ((MQTT_KEEPALIVE) >> 8);
         buffer[length++] = ((MQTT_KEEPALIVE) & 0xFF);
         length = mqttWriteString(id,buffer,length);
         if (willTopic) {
            length = mqttWriteString(willTopic,buffer,length);
            length = mqttWriteString(willMessage,buffer,length);
         }

         if(user != NULL) {
            length = mqttWriteString(user,buffer,length);
            if(pass != NULL) {
               length = mqttWriteString(pass,buffer,length);
            }
         }
         
   mqttWrite(MQTTCONNECT,buffer,length-5);
         
         lastInActivity = lastOutActivity = millis();
         
         while (!TCPRxLen ( Socket )) {
            unsigned long t = millis();
            if (t-lastInActivity > MQTT_KEEPALIVE*1000UL) {
               TCPClientClose ( Socket );
               return FALSE;
            }
         }
         uint8_t llen;
         uint16_t len = mqttReadPacket(&llen);
         
         if (len == 4 && buffer[3] == 0) {
            lastInActivity = millis();
            pingOutstanding = false;
            return true;
         }
      }
      TCPClientClose ( Socket );
   }
   return false;
   

uint8_t mqttReadByte() {
   while(!TCPRxLen ( Socket )) {}
   return mqttRead();
}

mqttRead(){
 uint8_t b;
  if (!TCPRxLen ( Socket ))
    return -1;
    TCPRead ( Socket, &b, 1);
  return b;

}

uint16_t mqttReadPacket(uint8_t* lengthLength) {
   uint16_t len = 0;
   buffer[len++] = mqttReadByte();
   uint8_t multiplier = 1;
   uint16_t length = 0;
   uint8_t digit = 0;
   do {
      digit = mqttReadByte();
      buffer[len++] = digit;
      length += (digit & 127) * multiplier;
      multiplier *= 128;
   } while ((digit & 128) != 0);
   *lengthLength = len-1;
   for (uint16_t i = 0;i<length;i++)
   {
      if (len < MQTT_MAX_PACKET_SIZE) {
         buffer[len++] = mqttReadByte();
      } else {
         mqttReadByte();
         len = 0; // This will cause the packet to be ignored.
      }
   }

   return len;
}


boolean mqttLoop() {
   if (flagTCPisCON==TRUE) {
      unsigned long t = millis();
      if ((t - lastInActivity > MQTT_KEEPALIVE*1000UL) || (t - lastOutActivity > MQTT_KEEPALIVE*1000UL)) {
         if (pingOutstanding) {
            TCPClientClose ( Socket );
            return false;
         } else {
            buffer[0] = MQTTPINGREQ;
            buffer[1] = 0;
            TCPWrite ( Socket, buffer, 2 );
            lastOutActivity = t;
            lastInActivity = t;
            pingOutstanding = true;
         }
      }
      if (TCPRxLen ( Socket )) {
         uint8_t llen;
         uint16_t len = mqttReadPacket(&llen);
         if (len > 0) {
            lastInActivity = t;
            uint8_t type = buffer[0]&0xF0;
            if (type == MQTTPUBLISH) {
               if (callback) {
                  uint16_t tl = (buffer[llen+1]<<8)+buffer[llen+2];
                  char topic[tl+1];
                  for (uint16_t i=0;i<tl;i++) {
                     topic[i] = buffer[llen+3+i];
                  }
                  topic[tl] = 0;
                  // ignore msgID - only support QoS 0 subs
                  uint8_t *payload = buffer+llen+3+tl;
                 // ?? callback(topic,payload,len-llen-3-tl);
               }
            } else if (type == MQTTPINGREQ) {
               buffer[0] = MQTTPINGRESP;
               buffer[1] = 0;
               TCPWrite ( Socket, buffer, 2) );
            } else if (type == MQTTPINGRESP) {
               pingOutstanding = false;
            }
         }
      }
      return true;
   }
   return false;
}

boolean mqttPublish_P(char* topic, uint8_t* PROGMEM payload, unsigned int plength, boolean retained) {
   uint8_t llen = 0;
   uint8_t digit;
   int rc;
   uint16_t tlen;
   int pos = 0;
   int i;
   uint8_t header;
   unsigned int len;
   
   if (!TCPRxLen ( Socket )) {
      return false;
   }
   
   tlen = strlen(topic);
   
   header = MQTTPUBLISH;
   if (retained) {
      header |= 1;
   }
   buffer[pos++] = header;
   len = plength + 2 + tlen;
   do {
      digit = len % 128;
      len = len / 128;
      if (len > 0) {
         digit |= 0x80;
      }
      buffer[pos++] = digit;
      llen++;
   } while(len>0);
   
   pos = mqttWriteString(topic,buffer,pos);
   
   rc += TCPWrite ( Socket, buffer, pos );
   
   for (i=0;i<plength;i++) {
      rc += TCPWrite ( Socket, (char)pgm_read_byte_near(payload + i), 1 );
   }
   
   lastOutActivity = millis();
   return rc == len + 1 + plength;
}

boolean mqttWite(uint8_t header, uint8_t* buf, uint16_t length) {
   uint8_t lenBuf[4];
   uint8_t llen = 0;
   uint8_t digit;
   uint8_t pos = 0;
   uint8_t rc;
   uint8_t len = length;
   do {
      digit = len % 128;
      len = len / 128;
      if (len > 0) {
         digit |= 0x80;
      }
      lenBuf[pos++] = digit;
      llen++;
   } while(len>0);

   buf[4-llen] = header;
   for (int i=0;i<llen;i++) {
      buf[5-llen+i] = lenBuf[i];
   }
   rc = TCPWrite ( Socket, buf+(4-llen), length+1+llen);
   
   lastOutActivity = millis();
   return (rc == 1+llen+length);
}

boolean mqttSubscribe(char* topic) {
   if (flagTCPisCON==TRUE) {
      // Leave room in the buffer for header and variable length field
      uint16_t length = 5;
      nextMsgId++;
      if (nextMsgId == 0) {
         nextMsgId = 1;
      }
      buffer[length++] = (nextMsgId >> 8);
      buffer[length++] = (nextMsgId & 0xFF);
      length = mqttWriteString(topic, buffer,length);
      buffer[length++] = 0; // Only do QoS 0 subs
      return mqttWrite(MQTTSUBSCRIBE|MQTTQOS1,buffer,length-5);
   }
   return false;
}

boolean mqttUnsubscribe(char* topic) {
   if (cflagTCPisCON==TRUE) {
      uint16_t length = 5;
      nextMsgId++;
      if (nextMsgId == 0) {
         nextMsgId = 1;
      }
      buffer[length++] = (nextMsgId >> 8);
      buffer[length++] = (nextMsgId & 0xFF);
      length = writeString(topic, buffer,length);
      return mqttWrite(MQTTUNSUBSCRIBE|MQTTQOS1,buffer,length-5);
   }
   return false;
}

void mqttDisconnect() {
   buffer[0] = MQTTDISCONNECT;
   buffer[1] = 0;
   TCPWrite ( Socket, buffer,2);
   TCPClientClose ( Socket );
   lastInActivity = lastOutActivity = millis();
}

 
uint16_t mqttWriteString(char* string, uint8_t* buf, uint16_t pos) {
   char* idp = string;
   uint16_t i = 0;
   pos += 2;
   while (*idp) {
      buf[pos++] = *idp++;
      i++;
   }
   buf[pos-i-2] = (i >> 8);
   buf[pos-i-1] = (i & 0xFF);
   return pos;
}

/*
boolean mqttConnected() {
   boolean rc;
   if (_client == NULL ) {
      rc = false;
   } else {
      rc = (int)_client->connected();
      if (!rc) _client->stop();
   }
   return rc;
}
 */

