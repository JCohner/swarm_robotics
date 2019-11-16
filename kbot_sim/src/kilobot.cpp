#pragma once
#include "kilolib.h"

class mykilobot : public kilobot
{

	unsigned char distance;
	message_t out_message;
	int rxed=0;
	float theta;


	int msrx=0;

	//main loop
	void loop()
	{	
		// printf("angle to light = %f , angle to last robot message = %f \n\r",angle_to_light,theta); 
		if (id == 0){
			set_color(RGB(1,1,1));
		} else if (id == 1){
			set_color(RGB(1,0,1));
		} else if (id == 2){
			set_color(RGB(1,0,0));
		}

	}

	//executed once at start
	void setup()
	{
		
		out_message.type = NORMAL;
		out_message.crc = message_crc(&out_message);
		printf("my id is: %d\n\n", id);

		
	}

	//executed on successfull message send
	void message_tx_success()
	{
		//set_color(RGB(1,0,0));
		
	}

	//sends message at fixed rate
	message_t *message_tx()
	{
		static int count = rand();
		count--;
		if (!(count % 10))
		{
			return &out_message;
		}
		return NULL;
	}

	//receives message
	void message_rx(message_t *message, distance_measurement_t *distance_measurement,float t)
	{
		distance = estimate_distance(distance_measurement);
		theta=t;
	}
};

