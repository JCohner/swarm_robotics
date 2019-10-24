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
		unsigned short x;
		unsigned short y;
		unsigned char h_count;
		unsigned short seed_list[2];
		unsigned char seed_count;
	};
	mydata my_info;
	//main loop
	void loop()
	{
		if(rxed==1)
		{
			// out_message.data[0] = (rid & 0xff00) >> 8; //id high
			// out_message.data[1] = rid & 0x00ff; //id low
    		rxed=0;
		}
		
		//update message
		// out_message.type = NORMAL;
		// out_message.data[0] = (rid & 0xff00) >> 8; //id high
		// out_message.data[1] = rid & 0x00ff; //id low
		// out_message.data[2] = keeper.lowest;
		// out_message.crc = message_crc(&out_message);


	}

	//executed once at start
	unsigned short rid;
	void setup()
	{
		//already has a random id inhereted from the robot class ask if we can expand this number
		id = id & 0xffff;
		rid = id;
		if (rid == 0xffff || rid == 0xfffe) {
			// execute seed logic
			printf("seed id is: %X\n", rid);
			set_color(RGB(0,0,1));

			my_info.seed_list[my_info.seed_count++] = rid;
			my_info.h_count = 0;
			//sets type to 1
			out_message.type = NORMAL;
			out_message.data[0] = (rid & 0xff00) >> 8; //highbits of id
			out_message.data[1] = rid & 0x00ff; //lowbits of id
			
			unsigned short x_short = (unsigned short)  pos[0];
			unsigned short y_short = (unsigned short) pos[1];

			out_message.data[2] = (x_short & 0xff00) >> 8; //xpos hi
			out_message.data[3] = (x_short & 0x00ff); //xpos low
			out_message.data[4] = (y_short & 0xff00) >> 8; //ypos hi
			out_message.data[5] = y_short & 0x00ff; //ypos low
			out_message.data[6] = my_info.h_count + 1;
			out_message.crc = message_crc(&out_message);
		} else if (rid > 0) {
			//execute member logic

			set_color(RGB(1,1,1));
		}
		
		
		
		// unsigned short test_x = (out_message.data[2] << 8) | (out_message.data[3] & 0xff);
		// unsigned short test_y = (out_message.data[4] << 8) |(out_message.data[5] & 0xff);

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
		//distance = estimate_distance(distance_measurement);
		if (message->data[6] == 1){
			set_color(RGB(1,0,0));
		}
		
		// if (message->data[1] > keeper.highest){
		// 	keeper.highest = message->data[1];
		// 	//set_color(RGB(1,0,0));
		// }

		// if (message->data[2] < keeper.lowest){
		// 	keeper.lowest = message->data[2];
		// 	//set_color(RGB(1,0,0));
		// }

		rxed=1;
	}
};
