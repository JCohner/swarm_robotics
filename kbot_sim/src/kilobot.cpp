#pragma once
#include "kilolib.h"

class mykilobot : public kilobot
{
	unsigned short NEIGHBOR_RANGE = 500;
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
	int my_rad = 16;
	float neighb_bias_angle;
	float neighb_bias_dist;
	double angle_to_cohesion;
	double desired_angle;
	float k = 1;
	double mag_repulse;
	double angie;
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

		
		/*perform calculations neccesarry for cohesion, alignment, repulsion, and migration*/
		//cohession, alignment, repulsion calculation
		double x_average = my_data.my_x;
		double y_average = my_data.my_y;
		double theta_average = my_data.my_heading;
		angie = 0;
		int num_neighbors = 0; //keeps track of inrange neighbors
		for (int i = 0; i < my_data.neighbor_index; i++){
			//only consider boids in range
			if (my_data.neighbor_list[i].distance < NEIGHBOR_RANGE){
				num_neighbors++;
				//calculate COM for cohesion
				x_average += my_data.neighbor_list[i].x;
				y_average += my_data.neighbor_list[i].y;

				//calculate average heading for alignemnt
				theta_average += my_data.neighbor_list[i].theta;
				
				//calculate average repulsion offset for repulsion
				mag_repulse = k * (NEIGHBOR_RANGE - my_data.neighbor_list[i].distance + 64)/NEIGHBOR_RANGE; //get fractional value for magnitude of repulsion based on distance
				angie += k * PI * mag_repulse;

			}
		}
		//convert our desired cohesion position to an angle
		x_average = x_average / (num_neighbors + 1);
		y_average = y_average / (num_neighbors + 1);
		if (num_neighbors){
			angle_to_cohesion = atan2((double)(y_average - my_data.my_y),(double) (x_average - my_data.my_x));
		} else{
			angle_to_cohesion = 0;
		}
		//get avearge of alignment and repulsion
		theta_average = theta_average / (num_neighbors + 1);
		angie = angie / (num_neighbors + 1);
		// printf("angle to light: %f\nrepulsion angle: %f\nangle to cohesion: %f\naverage heading: %f\n", angle_to_light, angie, angle_to_cohesion, theta_average);

		//calculatr attractiveness to light source
		dist_error = sqrt(pow((my_data.my_x - 1200),2) + pow((my_data.my_y - 1200),2));
		mag_repulse = dist_error/1200.0;
		// printf("mag repulse for migration is: %f\n", mag_repulse);
		angle_to_light = angle_to_light * mag_repulse;

		// printf("desired angle is: %f\n\n", desired_angle);

		curr_t = kilo_ticks;

		/*Now we want to align our boid with the average theta, move our boid towards the average x,y and reppel our boid away from angie*/
		desired_angle = ((2) * angle_to_light + (1.4) * angie + (0.4) * angle_to_cohesion + (0.2) * theta_average)/4;
		
		error = desired_angle / (2 * PI);

		if ((curr_t >= prev_t + t1) && (curr_t <= prev_t + t1 + t2) && (!angle_flag)) {
			if (error < 0){
				motorR = 50;
				motorL = 0;
			} else {
				motorR = 0;
				motorL = 50;
			}
			// t2 = (int)(5000 * abs(error));
		}
		else if ((curr_t >= prev_t + t1) && (curr_t <= prev_t + t1 + t3) && (angle_flag)){
			if (abs(error) > 0.50){
				// t2 = 50;
				// set_color(RGB(1,1,1));
				angle_flag = 0;
			} else {
				motorR = 50;
				motorL = 50;
			}	
		}

		else {
			motorR = 0;
			motorL = 0;

			if ((curr_t >= prev_t + t1 + t2)){
				prev_t = curr_t;
				
				// t2 = (int)(50 * abs(error));
				// if (abs(error) < 0.05){
				// 	t2 = 0;
				// 	// set_color(RGB(1,0,1));
				// 	//set aligned to desired angle
				// 	angle_flag = 1;
				// }
			}


			// if (angle_flag){
			// 	t3 = 200 * dist_error;
			// 	if (dist_error < 50){
			// 		t3 = 0;
			// 	}
			// }
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
		if (!(count % 2))
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
			// printf("telling me theta is: %f\n", theta);
		}

		//update their heading and distance in our neighbor list
		my_data.neighbor_list[sender_index].theta = theta;

		my_data.neighbor_list[sender_index].distance = distance;
		my_data.neighbor_list[sender_index].x = (message->data[1] << 8) | message->data[2];
		my_data.neighbor_list[sender_index].y = (message->data[3] << 8) | message->data[4];

		message_flag = 1;
	}
};

