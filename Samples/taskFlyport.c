#include "taskFlyport.h"
#include "mqtt.h"



void FlyportTask()
{
	WFConnect(WF_DEFAULT);
	while (WFStatus != CONNECTED);
	UARTWrite(1,"Flyport connected... hello world!\r\n");
    
   
	while(1)
	{
	mqttLoop();	
	}
  
}
