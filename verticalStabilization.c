//Compile => sudo gcc verticalStabilization.c /usr/include/pi-bno055/i2c_bno055.c -lwiringPi -o verticalStabilization

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <wiringPi.h>
#include <pi-bno055/getbno055.h>
#include <string.h>

int main()
{
	
	float pid_pitch = 0, pid_roll = 0, error_pitch = 0, error_roll = 0, previous_pitch_error = 0, previous_roll_error = 0, pwm_pitch = 0, pwm_roll = 0;
	float pitch = 0, roll = 0, oldPitch = 0, oldRoll = 0;
	float pid_pitch_p = 0;
	float pid_pitch_i = 0;
	float pid_pitch_d = 0;

	float pid_roll_p = 0;
	float pid_roll_i = 0;
	float pid_roll_d = 0;

	//Üst
	float kp1 = 0.25;         //0.1
	float ki1 = 0.001;      //0.0011
	float kd1 = 0.5;		 //400
	
	int control = 0;

	//Alt
	float kp2 = 0.25;  //0.25    //0.36
	float ki2 = 0.001;  //0.007   //0.1
	float kd2 = 0.45;     //6      //50

	int freq = 40;
	float throttle1 = 192;
	float throttle2 = 129;

	float desired_pitch_angle = 0;
	float desired_roll_angle = 0;

	char telemetry[100], pitchCommand[100], rollCommand[100];
	char statu[20] = "";
	int lastTimeTelemetry = 0, lastTimePid = 0;
	int controlStatu = 0;
	
	get_i2cbus("/dev/i2c-1", "0x28");
	opmode_t newmode = ndof;
    struct bnoeul bnod;
    struct bnogyr bnod2;
    set_mode(newmode);
	
	
	//system("sudo /home/pi/Desktop/PiBits/ServoBlaster/user/servod --p1pins=11,13,16,18");
	
	//snprintf(pitchCommand, 100, "echo P1-16=%f > /dev/servoblaster", throttle2);
	//snprintf(rollCommand, 100, "echo P1-18=%f > /dev/servoblaster", throttle1);
	
	//system(pitchCommand);
	//system(rollCommand);
	
	
	
	while(controlStatu == 0){
		
		delay(100);
		
		//Statu verisini okumak için
		FILE *statuReadDosya = fopen("/home/pi/Desktop/TURKSAT/statuDatas.txt", "r");
		fscanf(statuReadDosya, "%s", statu);
		fclose(statuReadDosya);
		
		
		if(strcmp(statu, "Ayrılma") == 0){
			delay(1000);
			controlStatu = 1;
		}
	}
	
	
	
	
		while(controlStatu == 1)
		{
			
		
			get_eul(&bnod);
			
			roll = 0 - bnod.eul_roll;
			pitch =  0 - bnod.eul_pitc;
			
			
			
			if(control > 20){
				
				if(oldPitch + 6 < pitch || oldPitch -6 > pitch){
					pitch = oldPitch;
				}
				if(oldRoll + 6 < roll || oldRoll -6 > roll){
					roll = oldRoll;
				}
			}
			
			control++;
			
			
			if(pitch > 180 || pitch < -180){
				pitch = oldPitch;
			}
			if(roll > 180 || roll < -180){
				roll = oldRoll;
			}
			
			
			oldPitch = pitch;
			oldRoll = roll;
				
			if(millis() > lastTimePid + freq)
			
			{
				lastTimePid = millis();
				
				pid_pitch_p = kp1*pitch; 
				pid_roll_p = kp2*roll;
				
				
				if((-2 < pitch && pitch < 2))
				{
				  pid_pitch_i = pid_pitch_i + (ki1*pitch); 
				}
				else{
					pid_pitch_i = 0;
				}
				
				if((-2 < roll && roll < 2))
				{
				  pid_roll_i = pid_roll_i + (ki2*roll); 
				}
				else{
					pid_roll_i = 0;
				}
			  
				pid_pitch_d = kd1*((pitch - previous_pitch_error)/freq);
				pid_roll_d = kd2*((roll - previous_roll_error)/freq);
			   
				pid_pitch = (pid_pitch_p + pid_pitch_i + pid_pitch_d);
				pid_roll = (pid_roll_p + pid_roll_i + pid_roll_d);
				
				if(pid_pitch < -8)
				{
				  pid_pitch = -8;
				}
				if(pid_pitch > 8)
				{
				  pid_pitch = 8;
				}
				
				if(pid_roll < -8)
				{
				  pid_roll = -8;
				}
				if(pid_roll > 8)
				{
				  pid_roll = 8;
				}

				
				pwm_pitch = throttle1 - pid_pitch;
				pwm_roll = throttle2 - pid_roll;
				
				
			}
			
			previous_pitch_error = pitch;
			previous_roll_error = roll;
			
			snprintf(pitchCommand, 100, "echo P1-16=%f > /dev/servoblaster && echo P1-18=%f > /dev/servoblaster", pwm_roll, pwm_pitch);
			
			system(pitchCommand);
			
			//system("echo P1-11=125 > /dev/servoblaster");
			//system("echo P1-13=125 > /dev/servoblaster");
			
			
			printf("PWM PITCH= %f\n", pwm_pitch);
			printf("PWM ROLL= %.1f\n", pwm_roll);
			printf("ROLL= %.1f\n", roll);
			printf("PITCH= %.1f\n\n", pitch);
			//printf("%.1f,", pitch);
			
			//printf("%d\n", millis());
			
			

		}
    
    
	
    return 0;
}
