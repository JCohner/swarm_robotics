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
			int i = 2;
			for (int x = 0; x < my_info.seed_count; x++){
				// printf("forwarding\n");
				out_message.data[i + x] = (my_info.seed_pos[x][0] & 0xff00) >> 8;
				out_message.data[i + x + 1] = my_info.seed_pos[x][0] & 0xff;
				out_message.data[i + x + 2] = (my_info.seed_pos[x][0] & 0xff00) >> 8;
				out_message.data[i + x + 3] = my_info.seed_pos[x][0] & 0xff;
				out_message.data[i + x + 4] = (my_info.seed_pos[x][1] & 0xff00) >> 8;
				out_message.data[i + x + 5] = my_info.seed_pos[x][1] & 0xff;
				out_message.data[i + x + 6] = my_info.h_count[x] + 1;
				printf("sending message with hopcount: %d\n", out_message.data[i + x + 6]);
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
			set_color(RGB(0,0,1));

			my_info.seed_list[my_info.seed_count++] = rid;
			my_info.my_x = x_short;
			my_info.my_y = y_short;
			my_info.seed_pos[my_info.seed_count][0] = x_short;
			my_info.seed_pos[my_info.seed_count][1] = y_short;
			my_info.h_count[0] = 0;
			my_info.h_count[1] = (unsigned char) 0xFE;
			//sets type to 1
			out_message.type = NORMAL;
			/*your id, seeds in this case*/
			out_message.data[0] = (rid & 0xff00) >> 8; //highbits of id
			out_message.data[1] = rid & 0x00ff; //lowbits of id
			
			out_message.data[2] = (rid & 0xff00) >> 8; //seed id hi
			out_message.data[3] = rid & 0x00ff; //seed id low

			/*seed 1 position*/
			out_message.data[4] = (x_short & 0xff00) >> 8; //xpos hi
			out_message.data[5] = (x_short & 0x00ff); //xpos low

			out_message.data[6] = (y_short & 0xff00) >> 8; //ypos hi
			out_message.data[7] = y_short & 0x00ff; //ypos low

			/*hopcount to that seed position*/
			out_message.data[8] = my_info.h_count[0] + 1;

			out_message.data[9] = 0x0; //seed2 id hi
			out_message.data[10] = 0x0; //seed2 id low
			out_message.data[11] = 0xff; //xpos hi
			out_message.data[12] = 0xff; //xpos low
			out_message.data[13] = 0xff; //ypos hi
			out_message.data[14] = 0xff; //ypos low
			out_message.data[15] = my_info.h_count[1];
			out_message.crc = message_crc(&out_message);
		} else if (rid > 0) {
			//execute member logic
			memset(my_info.h_count, (unsigned char) 0xFE, 2);

			out_message.type = NORMAL;
			out_message.data[0] = (rid & 0xff00) >> 8; //highbits of id
			out_message.data[1] = rid & 0x00ff; //lowbits of id
			
			// unsigned short x_short = (unsigned short)  pos[0];
			// unsigned short y_short = (unsigned short) pos[1];
			out_message.data[2] = 0x0; //seed 1 id hi
			out_message.data[3] = 0x0; //seed 1 id low
			out_message.data[4] = 0xff; //xpos hi
			out_message.data[5] = 0xff; //xpos low
			out_message.data[6] = 0xff; //ypos hi
			out_message.data[7] = 0xff; //ypos low
			out_message.data[8] = my_info.h_count[0];
			out_message.data[9] = 0x0; //seed 2 id hi
			out_message.data[10] = 0x0; //seed 2 id low
			out_message.data[11] = 0xff; //xpos hi
			out_message.data[12] = 0xff; //xpos low
			out_message.data[13] = 0xff; //ypos hi
			out_message.data[14] = 0xff; //ypos low
			out_message.data[15] = my_info.h_count[1];
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

		unsigned char hop = message->data[8];
		unsigned char hop2 = message->data[15];
		unsigned short seed1 = 0;
		unsigned short x_pos1;
		unsigned short y_pos1;
		unsigned short seed2 = 0;
		unsigned short x_pos2;
		unsigned short y_pos2;
		//listen to all hop fields that aren't uninitialized
		if (hop != 0xfe){
			seed1 = (message->data[2] << 8) | (message->data[3] & 0xff);
			x_pos1 = (message->data[4] << 8) | (message->data[5] & 0xff);
			y_pos1 = (message->data[6] << 8) | (message->data[7] & 0xff);

			int tracker = -1;
			if (seed1 == my_info.seed_list[0]){
				tracker = 0;
			} else if (seed1 == my_info.seed_list[1]){
				tracker = 1;
			} else {
				printf("heard from unexpected ")
			}

			if (hop != 0 && hop < )
		} 
		if (hop2 != 0xfe){
			seed2 = (message->data[9] << 8) | (message->data[10] & 0xff);
			x_pos2 = (message->data[11] << 8) | (message->data[12] & 0xff);
			y_pos2 = (message->data[13] << 8) | (message->data[14] & 0xff);
		} 
		
		//compare apples to apples
		//seeds align case


		// if (x == 0xffff | x == 0xfffe){

		if (hop < my_info.h_count[0] && hop != 0){
			if (hop % 2 == 1){
				set_color(RGB(1,0,1));
			} else if (hop % 2 == 0){
				set_color(RGB(0,1,0));
			} 
			my_info.h_count[0] = (unsigned char) hop;
			my_info.seed_pos[0][0] = x_pos1;
			my_info.seed_pos[0][1] = y_pos1;
			my_info.seed_list[0] = seed1; 			
			printf("hop count set to: %d\n", my_info.h_count[0]);
			my_info.seed_count++;
		}

		if (hop2 < my_info.h_count[2] && hop2 != 0){
			if (hop2 % 2 == 1){
				set_color(RGB(1,1,1));
			} else if (hop2 % 2 == 0){
				set_color(RGB(0,0,0));
			} 
			my_info.h_count[1] = (unsigned char) hop;
			my_info.seed_pos[1][0] = x_pos1;
			my_info.seed_pos[1][1] = y_pos1;
			my_info.seed_list[1] = seed1; 			
			printf("hop count set to: %d\n", my_info.h_count[1]);
			my_info.seed_count++;
		}


		rxed=1;
		
	}
};
