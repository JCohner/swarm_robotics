#pragma once
#include "kilolib.h"

class mykilobot : public kilobot
{

	unsigned char distance;
	message_t out_message;
	int rxed=0;
	float theta;
	char motorR = 50;
	char motorL = 45; 
	
	int curr_t;
	int prev_t = 0;
	int t1 = 100;
	int t2 = 50;
	double error;

	int state = 0;

	int msrx=0;

	//main loop
	void loop()
	{	
		curr_t = kilo_ticks;
		printf("prev t is: %d\t curr t is: %d\n", prev_t, curr_t);
		if ((curr_t >= prev_t + t1) && (curr_t <= prev_t + t1 + t2)) {
			motorR = 50;
		}  else{
			motorR = 0;
		}

		if (curr_t >= prev_t + t1 + t2){
			prev_t = curr_t;
			error = angle_to_light/ (2 * 3.14159265358979324);
			t2 = (int)(100 * abs(error));
			if (abs(error) < 0.05){
				t2 = 0;
				motorR = 0;
				set_color(RGB(1,0,1));
			}
		}
		printf("motorR is %d\n", motorR);
		spinup_motors();
		set_motors(motorR, 0);
		

		// printf("angle to light = %f , angle to last robot message = %f \n\r",angle_to_light,theta); 
		// if (id == 0){
		// 	set_color(RGB(1,1,1));
		// } else if (id == 1){
		// 	set_color(RGB(1,0,1));
		// } else if (id == 2){
		// 	set_color(RGB(1,0,0));
		// }
		
		//turn for kiloticks + 5 
		
		
		// printf("%d\n", kilo_ticks);

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

