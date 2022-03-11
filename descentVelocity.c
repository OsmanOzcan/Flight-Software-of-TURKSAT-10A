//Compile => sudo gcc descentVelocity.c -lwiringPi -lm -o descentVelocity

#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/types.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <wiringPi.h>


int main()
{
	
	float pid = 0, previous_error = 0, pwm = 0, error = 0;
	float yaw = 0, oldYaw = 0, differenceYaw = 0;
	float pid_p = 0;
	float pid_i = 0;
	float pid_d = 0;
	
	float kp = 0.6;
	float ki = 0.0005;
	float kd = 5;
	
	int freq = 40;
	float throttle = 165;
	
	
	char telemetry[100], command1[50], command2[50];
	int lastTimeTelemetry = 0, lastTimePid = 0;
	int controlStatu = 0;
	float pressure = 0, temperature = 0, velocity = 0, altitude = 0;
	char statu[20] = "";
	
	
	//system("echo P1-11=100 > /dev/servoblaster");
	//system("echo P1-13=100 > /dev/servoblaster");
	
	delay(10000);
	
	
	
	while(controlStatu == 0){
		
		delay(100);
		
		
		//Statu verisini okumak için
		FILE *statuReadDosya = fopen("/home/pi/Desktop/TURKSAT/statuDatas.txt", "r");
		fscanf(statuReadDosya, "%s", statu);
		fclose(statuReadDosya);
		
		
		if(strcmp(statu, "Ayrılma") == 0){
			delay(1500);
			controlStatu = 1;
			break;
		}
	}


	while (controlStatu == 1){
		
		
		//***********************************************************************
		
		printf("aaaaaaaaaaaaa\n");
		FILE *msDosya = fopen("/home/pi/Desktop/TURKSAT/MS5611datas.txt", "r");
		fscanf(msDosya, "%f %f %f %f", &temperature, &pressure, &altitude, &velocity);
		fclose(msDosya);
		
		error = -2 - velocity;
		
		if(millis() > lastTimePid + freq)
			{
				lastTimePid = millis();
				
				pid_p = kp*error;
				
			  
				if((-0.6 < error && error < 0.6))
				{
				  pid_i = pid_i + (ki*error); 
				}
				
				
				pid_d = kd*((error - previous_error)/freq);
				
				pid = (pid_p + pid_i + pid_d);
				
				if(pid < -10)
				{
				  pid = -10;
				}
				if(pid > 10)
				{
				  pid = 10;
				}

				
				pwm = throttle + pid;
				

				
			}
			
			previous_error = error;
			
			snprintf(command1, 50, "echo P1-11=%f > /dev/servoblaster", pwm);
			//snprintf(command2, 50, "echo P1-13=%f > /dev/servoblaster", pwm);
			
			//system(command1);
			//system(command2);
			
			
			/*printf("HIZ =%.1f\n", roc/100.0);
			printf("ERROR =%.1f\n", error);*/
			printf("PWM = %f\n\n", pwm);
			
			//printf("%.1f ", roc/100.0);
			
			FILE *motorDosya = fopen("/home/pi/Desktop/TURKSAT/motorPwm.txt", "w");
			fprintf(motorDosya, "%f", pwm);
			fclose(motorDosya);
			
					
	}
	return 0;
}
