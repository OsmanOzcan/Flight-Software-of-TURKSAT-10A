//Compile => sudo gcc verticalStabilization.c /usr/include/pi-bno055/i2c_bno055.c -lwiringPi -o verticalStabilization

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <wiringPi.h>
#include <pi-bno055/getbno055.h>

int main()
{
	
	float throttlePitch = 140, throttleRoll = 131;
	
	float kp = 0.25, kd = 0.9, ki = 0.001;
	float pid_pitch = 0, pid_roll = 0, pwm_pitch = 0, pwm_roll = 0;
	float pid_pitch_p = 0, pid_pitch_d = 0, pid_pitch_i = 0, pid_roll_p = 0, pid_roll_d = 0, pid_roll_i = 0;
	float previous_pitch_error = 0, previous_roll_error = 0;
	
	int lastTimePid = 0;
	char pitchCommand[50], rollCommand[50];
	
	//BNO055 başlatma
	get_i2cbus("/dev/i2c-1", "0x28");
	opmode_t newmode = ndof;
    	struct bnoeul bnod;
    	set_mode(newmode);
	
	// run servo program
	system("sudo /home/pi/Desktop/PiBits/ServoBlaster/user/servod --p1pins=16,18");
	
	
	snprintf(pitchCommand, 50, "echo P1-16=%f > /dev/servoblaster", throttle1);
	snprintf(rollCommand, 50, "echo P1-18=%f > /dev/servoblaster", throttle2);
	
	system(pitchCommand);
	system(rollCommand);
		
	while(1)
	{
		get_eul(&bnod);
		roll = bnod.eul_roll;
		pitch = bnod.eul_pitc;
		
		
		if(millis() > lastTimePid + freq)
		{
			lastTimePid = millis();

			pid_pitch_p = kp*pitch; 
			pid_roll_p = kp*roll;

			if(-5 < pitch && pitch < 5)
			{
				pid_pitch_i = pid_pitch_i + (ki*pitch); 
			}

			if(-5 < roll && roll < 5)
			{
				  pid_roll_i = pid_roll_i + (ki*roll); 
			}

			pid_pitch_d = kd*(pitch - previous_pitch_error);
			pid_roll_d = kd*(roll - previous_roll_error);

			pid_pitch = (pid_pitch_p + pid_pitch_i + pid_pitch_d);
			pid_roll = (pid_roll_p + pid_roll_i + pid_roll_d);

			if(pid_pitch < -20)
			{
				  pid_pitch = -20;
			}
			if(pid_pitch > 20)
			{
				  pid_pitch = 20;
			}

			if(pid_roll < -20)
			{
				  pid_roll = -20;
			}
			if(pid_roll > 20)
			{
				  pid_roll = 20;
			}
		}



		previous_pitch_error = pitch;
		previous_roll_error = roll;

		snprintf(pitchCommand, 50, "echo P1-16=%f > /dev/servoblaster", pwm_pitch);
		snprintf(rollCommand, 50, "echo P1-18=%f > /dev/servoblaster", pwm_roll);

		system(pitchCommand);
		system(rollCommand);
		
	}
    	
	return 0;
}
