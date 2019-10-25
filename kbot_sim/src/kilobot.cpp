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
				out_message.data[i + x + 2] = (my_info.seed_pos[x][1] & 0xff00) >> 8;
				out_message.data[i + x + 3] = my_info.seed_pos[x][1] & 0xff;
				out_message.data[i + x + 4] = my_info.h_count[x] + 1;
				// printf("sending message with hopcount: %d\n", out_message.data[i + x + 4]);
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
			
			/*seed 1 position*/
			out_message.data[2] = (x_short & 0xff00) >> 8; //xpos hi
			out_message.data[3] = (x_short & 0x00ff); //xpos low

			out_message.data[4] = (y_short & 0xff00) >> 8; //ypos hi
			out_message.data[5] = y_short & 0x00ff; //ypos low

			/*hopcount to that seed position*/
			out_message.data[6] = my_info.h_count[0] + 1;
			out_message.data[7] = 0xff; //xpos hi
			out_message.data[8] = 0xff; //xpos low
			out_message.data[9] = 0xff; //ypos hi
			out_message.data[10] = 0xff; //ypos low
			out_message.data[11] = my_info.h_count[1];
			out_message.crc = message_crc(&out_message);
		} else if (rid > 0) {
			//execute member logic
			memset(my_info.h_count, (unsigned char) 0xFE, 2);

			out_message.type = NORMAL;
			out_message.data[0] = (rid & 0xff00) >> 8; //highbits of id
			out_message.data[1] = rid & 0x00ff; //lowbits of id
			
			// unsigned short x_short = (unsigned short)  pos[0];
			// unsigned short y_short = (unsigned short) pos[1];

			out_message.data[2] = 0xff; //xpos hi
			out_message.data[3] = 0xff; //xpos low
			out_message.data[4] = 0xff; //ypos hi
			out_message.data[5] = 0xff; //ypos low
			out_message.data[6] = my_info.h_count[0];
			out_message.data[7] = 0xff; //xpos hi
			out_message.data[8] = 0xff; //xpos low
			out_message.data[9] = 0xff; //ypos hi
			out_message.data[10] = 0xff; //ypos low
			out_message.data[11] = my_info.h_count[1];
			out_message.crc = message_crc(&out_message);
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
		distance = estimate_distance(distance_measurement);
		// char hop = message->data[6];
		// if (hop < my_info.h_count){
		// 	my_info.h_count = hop;
		// 	// printf("hop: %d\n", hop);
		// 	// set_color(RGB(0,1,0));
		// 	if (hop == 1){
		// 		printf("slurp\n");
		// 		set_color(RGB(1,0,0));
		// 	}
		// }
		unsigned char hop = message->data[6];
		unsigned short sender = (message->data[0] << 8) | (message->data[1] & 0xff);
		// if (x == 0xffff | x == 0xfffe){
		if (hop < my_info.h_count[0] && hop != 0){
			if (hop % 2 == 1){
				set_color(RGB(1,0,1));
			} else if (hop % 2 == 0){
				set_color(RGB(0,1,0));
			} 
			my_info.h_count[0] = (unsigned char) hop;
			my_info.seed_pos[0][0] = message->data[4];
			my_info.seed_pos[0][1] = message->data[5];
			printf("hop count set to: %d\n", my_info.h_count[0]);
			my_info.seed_count++;
		}


		rxed=1;
		
	}
};
