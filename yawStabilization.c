#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <wiringPi.h>
#include <pi-bno055/getbno055.h>

int main()
{
	float pid_yaw = 0, previous_yaw_error = 0, pwm_yaw = 0;
	float yaw = 0, oldYaw = 0, differenceYaw = 0;
	float pid_yaw_p = 0;
	float pid_yaw_i = 0;
	float pid_yaw_d = 0;
	
	float kp = 1;
	float ki = 0.001;
	float kd = 5;
	
	int freq = 6;
	float throttle1 = 115;
	float throttle2 = 125;
	
	
	char telemetry[100], yawCommand[50];
	int lastTimeTelemetry = 0, lastTimePid = 0;
	
	get_i2cbus("/dev/i2c-1", "0x28");
	opmode_t newmode = ndof;
    	struct bnoeul bnod;
    	set_mode(newmode);
	
	// run servo program
	system("sudo /home/pi/Desktop/PiBits/ServoBlaster/user/servod --p1pins=11,13,16,18");
	
	
	system("echo P1-16=140 > /dev/servoblaster");
	system("echo P1-18=133 > /dev/servoblaster");
	
	system("echo P1-11=50 > /dev/servoblaster");
	system("echo P1-13=50 > /dev/servoblaster");
	
	delay(10000);
		
		while(1)
		{
		
			get_eul(&bnod);
			yaw = bnod.eul_head;
			differenceYaw = yaw - oldYaw;
			
			if(differenceYaw < -60){
				differenceYaw += 360;
			}
			if(differenceYaw > -60){
				differenceYaw -= 360;
			}
			
			oldYaw = yaw;
			
			
			if(millis() > lastTimePid + freq)
			{
				lastTimePid = millis();
				
				pid_yaw_p = kp*differenceYaw;
				
			  
				if((-2 < differenceYaw && differenceYaw < 2))
				{
				  pid_yaw_i = pid_yaw_i + (ki*differenceYaw); 
				}
				
				
				pid_yaw_d = kd*((differenceYaw - previous_yaw_error)/freq);
				
				pid_yaw = (pid_yaw_p + pid_yaw_i + pid_yaw_d);
				
				if(pid_yaw < -20)
				{
				  pid_yaw = -20;
				}
				if(pid_yaw > 20)
				{
				  pid_yaw = 20;
				}

				;
				pwm_yaw = throttle2 + pid_yaw;
				

				
			}
			
			previous_yaw_error = differenceYaw;
			
			snprintf(yawCommand, 50, "echo P1-13=%f > /dev/servoblaster", pwm_yaw);
			
			system(yawCommand);
			system("echo P1-11=115 > /dev/servoblaster");
			//system("echo P1-13=120 > /dev/servoblaster");
			
			
			printf("DIFFERENCE YAW=%.1f\n", differenceYaw);
			printf("PWM YAW= %f\n\n", pwm_yaw);
			

		}
    
    
	
    return 0;
}
