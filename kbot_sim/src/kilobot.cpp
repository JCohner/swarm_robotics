#pragma once
#include "kilolib.h"

class mykilobot : public kilobot
{

	unsigned char distance =255;
	message_t out_message;

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
	//main loop
	void loop()
	{	
		if (!doOnce){
			setup_p2();
			doOnce = 1;
		}


		curr_t = kilo_ticks;
		neighb_bias_angle = theta + PI;
		neighb_bias_dist = distance;
		// printf("doinkstance is %d\n", distance);
		// printf("my_radoink_doink is %d\n", 2 *my_rad);
		if (neighb_bias_dist < 2 * my_rad) {
			mag_repulse = k * (2*my_rad - neighb_bias_dist);
			// printf("interference naysh! %f\n", mag_repulse);
		} else {
			mag_repulse = 0;
		}


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
		if (id == 0){
			my_rad = 25;
			set_color(RGB(1,1,1));
		} else if (id == 1){
			my_rad = 50;
			set_color(RGB(1,0,1));
		} else if (id == 2){
			my_rad = 100;
			set_color(RGB(1,0,0));
		}
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
		message_flag = 1;
	}
};

