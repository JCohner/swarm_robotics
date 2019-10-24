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
		//data structure where highest lowest in heard will live, initialize it with current id because it is both the highest and lowest id we have heard
		unsigned int highest;
		unsigned int lowest;
	};
	mydata keeper;
	//main loop
	void loop()
	{
		if(rxed==1)
		{
			if (keeper.lowest == id){
				set_color(RGB(0,0,1));
			} 
			if (keeper.highest == id){
				set_color(RGB(0,1,0));
			}
    		rxed=0;
		}
		

	
		//update message
		out_message.type = NORMAL;
		out_message.data[0] = id;
		out_message.data[1] = keeper.highest;
		out_message.data[2] = keeper.lowest;
		out_message.crc = message_crc(&out_message);


	}

	//executed once at start
	void setup()
	{
		//already has a random id inhereted from the robot class
		id=id&0xff;

		keeper.highest = id;
		keeper.lowest = id;
		//sets type to 1
		out_message.type = NORMAL;
		//but 0 of 9 bits of data is bot ID
		out_message.data[0] = id;
		//could maybe have this as highest?
		out_message.data[1] = keeper.highest;
		//and this as lowest?
		out_message.data[2] = keeper.lowest;
		out_message.crc = message_crc(&out_message);
		
		//set to default color
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
		
		if (message->data[1] > keeper.highest){
			keeper.highest = message->data[1];
			set_color(RGB(1,0,0));
		}

		if (message->data[2] < keeper.lowest){
			keeper.lowest = message->data[2];
			set_color(RGB(1,0,0));
		}

		rxed=1;
	}
};
