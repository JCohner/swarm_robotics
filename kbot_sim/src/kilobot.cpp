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
		unsigned short my_x;
		unsigned short my_y;
		unsigned short seed_pos[2][2];
		unsigned char h_count[2];
		unsigned short seed_list[2];
		unsigned char seed_count;
	};
	mydata my_info;
	//main loop
	void loop()
	{
		if(rxed==1)
		{
			out_message.data[0] = (rid & 0xff00) >> 8; //id high
			out_message.data[1] = rid & 0x00ff; //id low
			for (int x = 0; x < 2; x++){
				out_message.data[x + 2] = my_info.h_count[x] + (unsigned char) 1;
				printf("sending message with hopcount: %d\n", out_message.data[3]);
			}
    		rxed=0;
		}

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
			unsigned short x_short = (unsigned short)  pos[0];
			unsigned short y_short = (unsigned short) pos[1];
			printf("I am at x: %d, y: %d\n", x_short, y_short);
			set_color(RGB(0,0,1));

			// Seed starts knowing where it is
			my_info.my_x = x_short;
			my_info.my_y = y_short;
			//all nodes know seed locations
			my_info.seed_pos[0][0] = 40;
			my_info.seed_pos[0][1] = 40;
			my_info.seed_pos[1][0] = 1280;
			my_info.seed_pos[1][1] = 40;
			
			if (rid & 0x0001){
				printf("seed 1\n");
				my_info.h_count[0] = 0;
				my_info.h_count[1] = (unsigned char) 0xFE;
				printf("my hope count is %d\n", my_info.h_count[0]);
			} else{
				printf("seed 2\n");
				my_info.h_count[0] = (unsigned char) 0xFE;
				my_info.h_count[1] = 0;
				printf("my hope count is %d\n", my_info.h_count[1]);
			}
			
			//sets type to 1
			out_message.type = NORMAL;
			/*your id, seeds in this case*/
			out_message.data[0] = (rid & 0xff00) >> 8; //highbits of id
			out_message.data[1] = rid & 0x00ff; //lowbits of id

			/*hopcount to that seed position*/
			out_message.data[2] = my_info.h_count[0] + 1;
			out_message.data[3] = my_info.h_count[1] + 1;

			out_message.crc = message_crc(&out_message);
		} else {
			//execute member logic
			// Seed starts knowing where it is
			my_info.my_x = 0;
			my_info.my_y = 0;
			//all nodes know seed locations
			my_info.seed_pos[0][0] = 0;
			my_info.seed_pos[0][1] = 0;
			my_info.seed_pos[1][0] = 32;
			my_info.seed_pos[1][1] = 0;
			my_info.h_count[0] = (unsigned char) 0xFE;
			my_info.h_count[1] = (unsigned char) 0xFE;

			out_message.type = NORMAL;
			out_message.data[0] = (rid & 0xff00) >> 8; //highbits of id
			out_message.data[1] = rid & 0x00ff; //lowbits of id
			out_message.data[2] = my_info.h_count[0] + 1;
			out_message.data[3] = my_info.h_count[1] + 1;
			out_message.crc = message_crc(&out_message);
			set_color(RGB(1,1,1));
		}
		
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

		unsigned char hop = message->data[2];
		unsigned char hop2 = message->data[3];
		// printf("heard h1: %d, h2: %d\n", hop, hop2);
		if (hop < my_info.h_count[0] && hop != 0){
			if (hop % 2 == 1){
				set_color(RGB(1,0,1));
			} else if (hop % 2 == 0){
				set_color(RGB(0,1,0));
			} 
			my_info.h_count[0] = (unsigned char) hop;			
			printf("hop count set to: %d\n", my_info.h_count[0]);
		}

		if (hop2 < my_info.h_count[1] && hop2 != 0){
			if (hop2 % 2 == 1){
				set_color(RGB(1,0,1));
			} else if (hop2 % 2 == 0){
				set_color(RGB(0,1,0));
			} 
			my_info.h_count[1] = (unsigned char) hop2;			
			printf("hop2 count set to: %d\n", my_info.h_count[1]);
		} 

		rxed=1;
		
	}
};
