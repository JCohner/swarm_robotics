#pragma once
#include "kilolib.h"

class mykilobot : public kilobot
{
	unsigned char distance;
	message_t out_message;
	int rxed=0;
	
	int motion=0;
	long int motion_timer=0;

	int msrx=0;
	struct mydata {
		unsigned int data1;
		unsigned int data2;
	};

	//main loop
	void loop()
	{
		
		
		if(rxed==1)
		{
			if(out_message.data[0]%2==0)
			{
			set_color(RGB(1,0,0));
			}
			else
			{
			set_color(RGB(0,1,0));
    			
			}
    			rxed=0;
		}
		

	
		//update message
		out_message.type = NORMAL;
		out_message.data[0] = id;
		out_message.data[1] = 0;
		out_message.data[2] = 0;
		out_message.crc = message_crc(&out_message);


	}

	//executed once at start
	void setup()
	{
		id=id&0xff;
		out_message.type = NORMAL;
		out_message.data[0] = id;
		out_message.data[1] = 0;
		out_message.data[2] = 0;
		out_message.crc = message_crc(&out_message);
		set_color(RGB(0,0,1));
		
	}

	//executed on successfull message send
	void message_tx_success()
	{
		//set_color(RGB(1,0,0));
		msrx=1;
	}

	//sends message at fixed rate
	message_t *message_tx()
	{
		static int count = rand();
		count--;
		if (!(count % 50))
		{
			return &out_message;
		}
		return NULL;
	}

	//receives message
	void message_rx(message_t *message, distance_measurement_t *distance_measurement)
	{
		distance = estimate_distance(distance_measurement);
		out_message.data[0] = message->data[0];
		out_message.data[1] = message->data[1];
		out_message.data[2] = message->data[2];
		rxed=1;
	}
};
