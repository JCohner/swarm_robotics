#pragma once
#include "kilolib.h"

class mykilobot : public kilobot
{
	unsigned char NEIGHBOR_RANGE = 200;
	unsigned char distance =255;
	short degrees;
	message_t out_message;

	struct neighbor_info{
		uint8_t id;
		uint16_t x;
		uint16_t y;
		double distance;
		double theta;
	};

	struct mydata
	{
		uint8_t my_id;
		uint8_t neighbor_index;
		uint16_t my_x;
		uint16_t my_y;
		double my_heading;
		neighbor_info neighbor_list[20]; //id, dist, angle
	};

	int rxed=0;
	float theta;
	char motorR = 50;
	char motorL = 45; 
	
	int curr_t;
	int prev_t = 0;
	int t1 = 100;
	int t2 = 50;
	int t3 = 500;
	double error;
	double dist_error;
	int angle_flag = 0;


	int doOnce = 0;
	int my_rad;
	float neighb_bias_angle;
	float neighb_bias_dist;
	float k = 1;
	float mag_repulse;
	float angie;
	int message_flag;

	int state = 0;

	int msrx=0;
	mydata my_data;
	//main loop
	void loop()
	{	
		if (!doOnce){
			doOnce = 1;
			setup_p2();
		}

		//update your own position and heading
		my_data.my_x = (uint16_t) pos[0];
		my_data.my_y = (uint16_t) pos[1];
		my_data.my_heading = angle_to_light;

		//package out message with your position and heading
		out_message.data[1] = (my_data.my_x & 0xff00) >> 8;
		out_message.data[2] = my_data.my_x & 0x00ff;
		out_message.data[3] = (my_data.my_y & 0xff00) >> 8;
		out_message.data[4] = my_data.my_y & 0x00ff;

		

		curr_t = kilo_ticks;
		neighb_bias_angle = theta + PI;
		neighb_bias_dist = distance;

		// if (neighb_bias_dist < 2 * my_rad) {
		// 	mag_repulse = k * (2*my_rad - neighb_bias_dist);
		// 	// printf("interference naysh! %f\n", mag_repulse);
		// } else {
		// 	mag_repulse = 0;
		// }

		mag_repulse = 0;

		if (mag_repulse){
			angie = ((2 - k) * angle_to_light + k *neighb_bias_angle)/2; //Can be improved
			error = angie/ (2 * PI);
		} else{
			// printf("doing this\n");
			error = angle_to_light/ (2 * PI);	
		}
		
		// if (message_flag){
		// 	message_flag = 0;
		// 	distance = 255;
		// }
		
		dist_error = sqrt(pow((pos[0] - 1200),2) + pow((pos[1] - 1200),2));

		if ((curr_t >= prev_t + t1) && (curr_t <= prev_t + t1 + t2) && (!angle_flag)) {
			if (error < 0){
				motorR = 50;
				motorL = 0;
			} else {
				motorR = 0;
				motorL = 50;
			}
			// t2 = (int)(50 * abs(error));
		}
		else if ((curr_t >= prev_t + t1) && (curr_t <= prev_t + t1 + t3) && (angle_flag)){
			if (abs(error) > 0.05){
				// t2 = 50;
				// set_color(RGB(1,1,1));
				angle_flag = 0;
			} else {
				motorR = 50;
				motorL = 50;
			}	
		}

		else {
			if ((curr_t >= prev_t + t1 + t2)){
				prev_t = curr_t;
				
				t2 = (int)(50 * abs(error));
				if (abs(error) < 0.05){
					t2 = 0;
					// set_color(RGB(1,0,1));
					angle_flag = 1;
				}
			}
			motorR = 0;

			if (angle_flag){
				t3 = 200 * dist_error;
				if (dist_error < 50){
					t3 = 0;
				}
			}
		}

		// printf("motorR is %d\n", motorR);
		spinup_motors();
		set_motors(motorR, motorL);
	}

	void setup_p2(){
		//give yourself a random id, populate your heading with current heading
		my_data.my_x = pos[0];
		my_data.my_y = pos[1];
		my_data.my_id = rand() % 255;
		my_data.neighbor_index = 0;
		out_message.data[0] = my_data.my_id;
	}

	//executed once at start
	void setup()
	{
		out_message.type = NORMAL;
		out_message.crc = message_crc(&out_message);
		
		set_color(RGB(1,1,1));	
		spinup_motors();
	}

	//executed on successfull message send
	void message_tx_success()
	{
		// set_color(RGB(1,0,0));
		
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
		//figure out who you're hearing from
		unsigned char sender_id = message->data[0];
		unsigned char sender_index;
		bool fam_face_flag = 0;
		for (int i = 0; i < my_data.neighbor_index; i++){
			if (my_data.neighbor_list[i].id == sender_id){
				sender_index = i;
				fam_face_flag = 1;
				break;
			}
		}
		if (!fam_face_flag){
			printf("heard from a new person\n");
			my_data.neighbor_list[my_data.neighbor_index].id = sender_id;
			sender_index = my_data.neighbor_index;
			my_data.neighbor_index++;
		}

		//update their heading and distance in our neighbor list
		my_data.neighbor_list[sender_index].theta = theta;
		my_data.neighbor_list[sender_index].distance = distance;
		my_data.neighbor_list[sender_index].x = (message->data[1] << 8) | message->data[2];
		my_data.neighbor_list[sender_index].y = (message->data[3] << 8) | message->data[4];

		message_flag = 1;
	}
};

