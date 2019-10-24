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
				//set_color(RGB(0,0,1));
			} 
			if (keeper.highest == id){
				//set_color(RGB(0,1,0));
			}
    		rxed=0;
		}
		
		//update message
		out_message.type = NORMAL;
		out_message.data[0] = id & 0xff00; //id high
		out_message.data[1] = id & 0x00ff; //id low
		out_message.data[2] = keeper.lowest;
		out_message.crc = message_crc(&out_message);


	}

	//executed once at start
	void setup()
	{
		//already has a random id inhereted from the robot class ask if we can expand this number
		id = id & 0xffff;
		// printf("id #: %d\n", id);
		if (id == 0xffff || id == 0xfffe) {
			// printf("<0!!!!!!\n");
			set_color(RGB(0,0,1));
		} else if (id > 0) {
			// printf("im not <0!\n");
			set_color(RGB(1,1,1));
		}
		

		//sets type to 1
		out_message.type = NORMAL;
		out_message.data[0] = id & 0xff00; //highbits of id
		out_message.data[1] = id & 0x00ff; //lowbits of id
		
		unsigned short x_short = (unsigned short)  pos[0];
		unsigned short y_short = (unsigned short) pos[1];

		out_message.data[2] = (x_short & 0xff00) >> 8; //xpos
		out_message.data[3] = (x_short & 0x00ff); //ypos
		out_message.data[4] = (y_short & 0xff00) >> 8; //xpos
		out_message.data[5] = y_short & 0x00ff; //ypos
		out_message.crc = message_crc(&out_message);
		
		// printf("x: %f, y: %f\n", pos[0], pos[1]);
		// printf("x: %d, y: %d\n", x_short, y_short);
		// unsigned short test_x = (out_message.data[2] << 8) | (out_message.data[3] & 0xff);
		// unsigned short test_y = (out_message.data[4] << 8) |(out_message.data[5] & 0xff);
		// printf("x: %d, y: %d\n", test_x , test_y);
		// printf("\n");

		//set to default color
		// set_color(RGB(0,0,1));
		// if (id < 0){
		// 	printf("hey im negative!\n");
		// 	set_color(RGB(1,0,0));
		// }
		
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
			//set_color(RGB(1,0,0));
		}

		if (message->data[2] < keeper.lowest){
			keeper.lowest = message->data[2];
			//set_color(RGB(1,0,0));
		}

		rxed=1;
	}
};
