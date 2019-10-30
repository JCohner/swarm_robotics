#pragma once
#include "kilolib.h"
#include <stdint.h>

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
		unsigned short seed_pos[2][2];
		unsigned char h_count[2];
		unsigned short seed_list[2];
		unsigned char seed_count;
		int i;
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
				// printf("sending message with hopcount: %d\n", out_message.data[3]);
			}
    		rxed=0;
		}

		mulilateration();

	}

	void mulilateration(){
		if ((my_info.h_count[0] != 0xFE) && (my_info.h_count[1] != 0xFE)) {
			//first guess
			if (!my_info.i){
				//make our first guess based on which seed is closer
				// printf("making a first guess!\n");
				if (my_info.h_count[0] > my_info.h_count[1]){
					my_info.x = my_info.seed_pos[0][0];
					my_info.y = my_info.seed_pos[0][1];
				} else {
					my_info.x = my_info.seed_pos[1][0];
					my_info.y = my_info.seed_pos[1][1];
				}

				++my_info.i;
			}

			//every time, compute a better guess
			float dj0 = euc_dist(my_info.x, my_info.y , my_info.seed_pos[0][0], my_info.seed_pos[0][1]);
			float dj1 = euc_dist(my_info.x, my_info.y , my_info.seed_pos[1][0], my_info.seed_pos[1][1]);
			float dj0_hat = my_info.h_count[0] * radius * 7; 
			float dj1_hat = my_info.h_count[1] * radius * 7;
			float err0 = pow((dj0 - dj0_hat),2);
			float err1 = pow((dj1 - dj1_hat),2);
			float err = err0 + err1;

			//move about 4 directions until youve minimized error, assign minimum step as new x and y
			uint16_t new_x;
			uint16_t new_y;
			float new_err0;
			float new_err1;
			float new_err[4]; 

			//N
			if (my_info.y != 1280){
				new_x = my_info.x + (0 * 40);
				new_y = my_info.y + (1 * 40);

				new_err0 = pow((euc_dist(new_x, new_y, my_info.seed_pos[0][0], my_info.seed_pos[0][1]) - dj0_hat),2);
				new_err1 = pow((euc_dist(new_x, new_y, my_info.seed_pos[1][0], my_info.seed_pos[1][1]) - dj1_hat),2);

				new_err[0] = new_err0 + new_err1;
			} else {
				new_err[0] = INFINITY;
			}
				

			//E
			if (my_info.x != 1280){
				new_x = my_info.x + (1 * 40);
				new_y = my_info.y + (0 * 40);

				new_err0 = pow((euc_dist(new_x, new_y, my_info.seed_pos[0][0], my_info.seed_pos[0][1]) - dj0_hat),2);
				new_err1 = pow((euc_dist(new_x, new_y, my_info.seed_pos[1][0], my_info.seed_pos[1][1]) - dj1_hat),2);

				new_err[1] = new_err0 + new_err1;
			} else{
				new_err[1] = INFINITY;
			}
				

			//S
			if (my_info.y != 40){
				new_x = my_info.x + (0 * 40);
				new_y = my_info.y + (-1 * 40);

				new_err0 = pow((euc_dist(new_x, new_y, my_info.seed_pos[0][0], my_info.seed_pos[0][1]) - dj0_hat),2);
				new_err1 = pow((euc_dist(new_x, new_y, my_info.seed_pos[1][0], my_info.seed_pos[1][1]) - dj1_hat),2);

				new_err[2] = new_err0 + new_err1;

			} else{
				new_err[2] = INFINITY;
			}
			
			//W
			if (my_info.x != 40){
				new_x = my_info.x + (-1 * 40);
				new_y = my_info.y + (0 * 40);

				new_err0 = pow((euc_dist(new_x, new_y, my_info.seed_pos[0][0], my_info.seed_pos[0][1]) - dj0_hat),2);
				new_err1 = pow((euc_dist(new_x, new_y, my_info.seed_pos[1][0], my_info.seed_pos[1][1]) - dj1_hat),2);

				new_err[3] = new_err0 + new_err1;
			} else{
				new_err[3] = INFINITY;
			}
			
			uint8_t dir = 4; //i starts by pointing at error of current guess
			float min_err = err;
			for(int j = 0; j < 4; j++){
				if (new_err[j] < min_err){
					min_err = new_err[j];
					dir = j;
				}
			}
			float color1;
			float color2;
			switch(dir){
				case 0:
					//North case
					set_color(RGB(1,0,1));
					my_info.y = my_info.y + 40; 
					break;
				case 1:
					//East case
					set_color(RGB(0,1,1));
					my_info.x = my_info.x + 40;
					break;
				case 2:
					//South case
					set_color(RGB(1,1,0));
					my_info.y = my_info.y - 40;
					break;
				case 3:	
					//West case
					set_color(RGB(0,0,1));
					my_info.x = my_info.x - 40;
					break;
				case 4:
					// printf("no error!\n");
					color1 = (my_info.x)/(1280.0);
					color2 = my_info.y/1280.0;

					printf("I am at x: %d, y:%d\n", my_info.x, my_info.y);
					set_color(RGB(color1,0,color2));
					break;
				default:
					printf("eggs\n");
			}


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
			my_info.x = x_short;
			my_info.y = y_short;
			//all nodes know seed locations
			my_info.seed_pos[0][0] = 40;
			my_info.seed_pos[0][1] = 640;
			my_info.seed_pos[1][0] = 1280;
			my_info.seed_pos[1][1] = 640;

			my_info.i = 1; //does not need to guess about position
			
			if (rid & 0x0001){
				printf("seed 1\n");
				my_info.h_count[0] = 0;
				my_info.h_count[1] = (unsigned char) 0xFE;
				// printf("my hope count is %d\n", my_info.h_count[0]);
			} else{
				printf("seed 2\n");
				my_info.h_count[0] = (unsigned char) 0xFE;
				my_info.h_count[1] = 0;
				// printf("my hope count is %d\n", my_info.h_count[1]);
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
			my_info.x = 0xffff;
			my_info.y = 0xffff;
			my_info.i = 0; 
			//all nodes know seed locations
			my_info.seed_pos[0][0] = 40;
			my_info.seed_pos[0][1] = 40;
			my_info.seed_pos[1][0] = 1280;
			my_info.seed_pos[1][1] = 40;
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

	float euc_dist(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2){
		float ret_val;
		ret_val = sqrt(pow((x1 - x2), 2) + pow((y1 - y2),2));
		return ret_val;
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
			my_info.h_count[0] = hop;				
			// printf("hop count set to: %d, guessing im at %d,%d\n", my_info.h_count[0], my_info.x, my_info.y);
		}

		if (hop2 < my_info.h_count[1] && hop2 != 0){
			if (hop2 % 2 == 1){
				set_color(RGB(1,0,1));
			} else if (hop2 % 2 == 0){
				set_color(RGB(0,1,0));
			} 
			my_info.h_count[1] = hop2;			
			// printf("hop2 count set to: %d\n", my_info.h_count[1]);
		} 

		rxed=1;
		
	}
};
