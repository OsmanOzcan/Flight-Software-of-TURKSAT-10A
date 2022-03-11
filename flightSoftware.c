// Compile => sudo gcc flightSoftware.c /usr/include/pi-bno055/i2c_bno055.c ina219/raspberrypi4b_driver_ina219_interface.c ina219/iic.c ina219/driver_ina219_basic.c ina219/driver_ina219.c -lm -lgps -l wiringPi  -o flightSoftware

#include <wiringPi.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/types.h>
#include <fcntl.h>
#include <math.h>



//--------------------------------------------------Kullanılan fonksiyonlar----------------------------------------------------
void timeFunction(time_t now, int *year, int *month, int *day, int *hour, int *minute, int *second);
void videoPacketInfo(char videoPacket[6]);
void batteryVoltage(float *current, float *voltage, int *controlCamera);
void rotation(int *rotationNumber, float *yaw, float *differenceYaw, float *oldYaw, int *singleInput, int *numberControl, int *directionControl, int *directionControl2);
void bnoFunction(float *yaw, float *pitch, float *roll);
void msFunction(float *temperature, float *pressure, float *altitude, float *velocity, float *startAltitude, int *altitudeControl);
unsigned int PROM_read(int DA, char PROM_CMD);
long CONV_read(int DA, char CONV_CMD);
void satelliteStatu(char statu[20], float *altitude, float *velocity, int *statuControl);
void task(char statu[20], int *lastTimeMOSFET, int *lastTimeMOSFETControl, int *controlMOSFET, int *lastTimeBUZZER, int *lastTimeBUZZERControl);
void stopCoaxiel();
//------------------------------------------------------------------------------------------------------------------------------


int main(){
	const unsigned short teamID = 59637;
	int packetNumber = -2;
		
	//Zaman tanımlamaları
	int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;
	//GPS tanımlamaları
	float gpsLatitude = 0, gpsLongitude = 0, gpsAltitude = 0;
	//MS5611 tanımlamaları
	float temperature = 0, pressure = 0, altitude = 0, velocity = 0, startAltitude = 0;
	int altitudeControl = 0;
	//BNO055 tanımlamaları
	float yaw = 0, pitch = 0, roll = 0;
	//INA219 tanımlamaları
	float current = 0, voltage = 0;
	int controlCamera = 0;
	//Uydu statüsü
	char statu[20] = "Beklemede";
	int statuControl = 0;
	//Video paketi aktarım bilgisi
	char videoPacket[6] = "Hayır";
	//Dönüş sayısı
	int rotationNumber = 0;
	float differenceYaw = 0, oldYaw = 0;
	int singleInput = 1, directionControl = 0, directionControl2 = 0, numberControl = 1;
	//Tarih tanımlamaları
	time_t now;
	//Yer istasyonundan bilgi
	char fromGCS[2] = "";
	//Genel
	int lastTime = 0, lastTimeMOSFET=0, lastTimeMOSFETControl = 1, controlMOSFET = 1, lastTimeBUZZER=0, lastTimeBUZZERControl = 1, controlBUZZER = 1;
	char timeTelemetry[21], telemetry[200], mysqlTelemetry[300];
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------
	//---------------------------------------------------------------------------------------------------------------------------------------------------
	
	
	
	//Son veriyi okuma
	FILE *lastDataReadDosya = fopen("/home/pi/Desktop/TURKSAT/OOPROM.txt", "r");
	fscanf(lastDataReadDosya, "%d %d %f %d %d %d", &packetNumber, &rotationNumber, &startAltitude, &altitudeControl, &statuControl, &controlMOSFET);
	fclose(lastDataReadDosya);
	
	
	//GPIO pinleri
	wiringPiSetup();
	
	pinMode(10, OUTPUT);
	pinMode(24, OUTPUT);
	pinMode(25, OUTPUT);
	digitalWrite(10, LOW);
	digitalWrite(24, LOW);
	digitalWrite(25, LOW);
	
	
	//Motorları başlatma
	system("sudo killall servod");
	system("sudo /home/pi/Desktop/PiBits/ServoBlaster/user/servod --p1pins=11,13,16,18");
	
	system("echo P1-11=100 > /dev/servoblaster");
	system("echo P1-13=100 > /dev/servoblaster");
	

	while(1){
		
		//GPS verisini okumak için
		FILE *gpsReadDosya = fopen("/home/pi/Desktop/TURKSAT/GPSdatas.txt", "r");
		fscanf(gpsReadDosya, "%f %f %f", &gpsLatitude, &gpsLongitude, &gpsAltitude);
		fclose(gpsReadDosya);
		
		
		//BNO055 verisini okumak için
		FILE *bno055ReadDosya = fopen("/home/pi/Desktop/TURKSAT/BNO055datas.txt", "r");
		fscanf(bno055ReadDosya, "%f %f %f", &yaw, &pitch, &roll);
		fclose(bno055ReadDosya);
		
		
		//INA219 verisini okumak için
		FILE *ina219ReadDosya = fopen("/home/pi/Desktop/TURKSAT/ina219/INA219datas.txt", "r");
		fscanf(ina219ReadDosya, "%f", &current);
		fclose(ina219ReadDosya);
		
		
		//MS5611 verisini okumak için
		FILE *ms5611ReadDosya = fopen("/home/pi/Desktop/TURKSAT/MS5611datas.txt", "r");
		fscanf(ms5611ReadDosya, "%f %f %f %f", &temperature, &pressure, &altitude, &velocity);
		fclose(ms5611ReadDosya);
		
		
		//Statü yazdırma
		FILE *statuDosya = fopen("/home/pi/Desktop/TURKSAT/statuDatas.txt", "w");
		fprintf(statuDosya, "%s", statu);
		fclose(statuDosya);
		
		
		
		timeFunction(now, &year, &month, &day, &hour, &minute, &second);
		videoPacketInfo(videoPacket);
		batteryVoltage(&current, &voltage, &controlCamera);
		msFunction(&temperature, &pressure, &altitude, &velocity, &startAltitude, &altitudeControl);
		rotation(&rotationNumber, &yaw, &differenceYaw, &oldYaw, &singleInput, &numberControl, &directionControl, &directionControl2);
		satelliteStatu(statu, &altitude, &velocity, &statuControl);
		task(statu, &lastTimeMOSFET, &lastTimeMOSFETControl, &controlMOSFET, &lastTimeBUZZER, &lastTimeBUZZERControl);
		
		//printf("%d\n", millis());
		
		
		//1 Hz telemetri aktarımı
		if(millis() > lastTime+1000){
			lastTime = millis();
			packetNumber++;
			
			if(packetNumber >= 1){
				
				//SD karta telemetriyi kaydetmek için dosya açma
				FILE *telemetryDosya = fopen("/home/pi/Desktop/TURKSAT/telemetry.txt", "a");  // a => Altına ekle
				//Son veriyi kaydetmek için dosya açma
				FILE *lastDataWriteDosya = fopen("/home/pi/Desktop/TURKSAT/OOPROM.txt", "w");  // w => Üzerine yaz
				//GCS okumak için dosya açma
				FILE *GCSDosya = fopen("/home/pi/Desktop/TURKSAT/GCS.txt", "r");
				
				
				//Telemetri paketi oluşturma
				snprintf(timeTelemetry, 21, "%d/%d/%d - %d:%d:%d", day, month, year, hour, minute, second);
				snprintf(telemetry, 200, "%d,%d,%s,%.1f,%.1f,%.2f,%.1f,%.2f,%.6f,%.6f,%.1f,%s,%.2f,%.2f,%.2f,%d,%s", teamID, packetNumber, timeTelemetry, pressure, altitude, velocity, temperature, voltage, gpsLatitude, gpsLongitude, gpsAltitude, statu, pitch, roll, yaw, rotationNumber, videoPacket);
				
				//Ekrana yazdırma
				printf("%s\n", telemetry);
				
				//SD karta telemetri yazdırma
				fprintf(telemetryDosya, "%s\n", telemetry);
				fclose(telemetryDosya);
				
				
				//Son veriyi kaydetme
				fprintf(lastDataWriteDosya, "%d %d %f %d %d %d", packetNumber, rotationNumber, startAltitude, altitudeControl, statuControl, controlMOSFET);
				fclose(lastDataWriteDosya);
				
				
				//GCS okuma
				fscanf(GCSDosya, "%s", fromGCS);
				fclose(GCSDosya);
				
				
				//Yer istasyonundan gönderilen komutlara tepki verme 
				if(strcmp(fromGCS, "A") == 0){
					
					statuControl = 2;
					
					if(controlMOSFET){
						digitalWrite(24, HIGH);
						digitalWrite(25, HIGH);
							//MOSFET 2 saniye aktive ediliyor
						if(lastTimeMOSFETControl){
							lastTimeMOSFET = millis();
							lastTimeMOSFETControl = 0;
						}
							if(millis() > lastTimeMOSFET + 2000){
								digitalWrite(24, LOW);
								digitalWrite(25, LOW);
							
							
							controlMOSFET = 0;
						}
					}
					else{
						digitalWrite(24, LOW);
						digitalWrite(25, LOW);
					}
					
				}
				else if(strcmp(fromGCS, "B") == 0){
					
					
					system("echo P1-16=140 > /dev/servoblaster");
					system("echo P1-18=133 > /dev/servoblaster");
					system("echo P1-11=110 > /dev/servoblaster");
					system("echo P1-13=110 > /dev/servoblaster");
					
					//printf("motor ve servo acildi");
				}
				else if(strcmp(fromGCS, "C") == 0){
					
					stopCoaxiel();
					
					//printf("motor ve servo kapandi");
					
				}
			
			}
		}
	}
	
    return 0;
}


void timeFunction(time_t now, int *year, int *month, int *day, int *hour, int *minute, int *second){
	
	time(&now);
	struct tm *local = localtime(&now);
	
	*year = local->tm_year + 1900;
	*month = local->tm_mon + 1;
	*day = local->tm_mday;
	*hour = local->tm_hour;
	*minute = local->tm_min;
	*second = local->tm_sec;
}



void videoPacketInfo(char videoPacket[6]){
	if(access("/home/pi/video.mp4", F_OK) != -1){
		strcpy(videoPacket, "Evet");
	}
	else{
		strcpy(videoPacket, "Hayır");
	}
}




void batteryVoltage(float *current, float *voltage, int *controlCamera){
	
	*voltage = (*current/1000.0)*23;
	
	//*voltage = 3.87 + (rand() / (float) RAND_MAX)* (0.02);
	
	if(*voltage < 3.55 && *voltage > 3.05){
		
		*controlCamera++;
		
		if(*controlCamera > 100){
			system("sudo killall raspivid");
		}
		
	}
	
}




void rotation(int *rotationNumber, float *yaw, float *differenceYaw, float *oldYaw, int *singleInput, int *numberControl, int *directionControl, int *directionControl2){
	
  	*differenceYaw = *yaw - *oldYaw;
  	*oldYaw = *yaw;
	

	if(*yaw > 175 && *yaw < 185 && *singleInput){

		if((*numberControl%2)){

	 		if(*differenceYaw > 0){
				*directionControl = 1;
	      		}
	      		else{
				*directionControl = 0;
	      		}
	    	}
	    	else{

	      		if(*differenceYaw > 0){
				*directionControl2 = 1;
	      		}
	      		else{
				*directionControl2 = 0;
	      		}
	    	}

	    	if((*directionControl && *directionControl2) || !(*directionControl || *directionControl2) ){
			(*rotationNumber)++;
	    	}

	    	(*numberControl)++;

	    	*singleInput = 0;

	  }
	
	if(*yaw < 175 || *yaw > 185){
    		*singleInput = 1;
  	}
}




void msFunction(float *temperature, float *pressure, float *altitude, float *velocity, float *startAltitude, int *altitudeControl){
	
	
	if(*altitudeControl < 1000){                 // Ölçümlere başlandıktan sonra yüksek doğruluğu sağlamak için 
    		*startAltitude = *altitude;           // 20. veriden sonra başlangıç yüksekliğini hesaplıyor (yaklaşık X sn)
    		(*altitudeControl)++;
  	}
	
	*altitude = *altitude - *startAltitude;
	
	if(*altitude < 0){
    		*altitude = 0;
  	}
}






void satelliteStatu(char statu[20], float *altitude, float *velocity, int * statuControl){
	
	
	if(*altitude<5 && *velocity>=0 && *velocity<1 && *statuControl==0){
		strcpy(statu, "Beklemede");
	}
	else if(*altitude>=5 && *velocity>=1 && *statuControl == 0){
		strcpy(statu, "Yükselme");
	}
	else if(*altitude>60 && *velocity<1 && *velocity>-1 && *statuControl == 0){
		strcpy(statu, "Zirve");
	}
	else if(*altitude>50 && *velocity<=-2 && *statuControl<2){
		strcpy(statu, "Model Uydu İniş");
		*statuControl = 1;
	}
	else if(*altitude<50 && *altitude>40 && *velocity<-2 && *statuControl<3){
		strcpy(statu, "Ayrılma");
		*statuControl = 2;
	}
	else if(*altitude<395 && *altitude>225 && *velocity <-2){
		strcpy(statu, "Görev Yükü İniş");
		*statuControl = 2;
	}
	else if(*altitude<225 && *altitude>180 && *velocity>-3 && *velocity<3 && *statuControl == 2){
		strcpy(statu, "İrtifa Sabitleme");
		*statuControl = 2;
	}
	else if(*altitude<180 && *velocity <-2 && *statuControl == 2){
		strcpy(statu, "Görev Yükü 2. İniş");
		*statuControl = 2;
	}
	else if(*altitude<5 && *velocity < 1 && *velocity > -1 && *statuControl==2){
		strcpy(statu, "Kurtarma");
	}
}



void task(char statu[20], int *lastTimeMOSFET, int *lastTimeMOSFETControl, int *controlMOSFET, int *lastTimeBUZZER, int *lastTimeBUZZERControl){
	
	if(strcmp(statu, "Ayrılma") == 0 && *controlMOSFET){
		
		digitalWrite(24, HIGH);
		digitalWrite(25, HIGH);
		
		//MOSFET 2 saniye aktive ediliyor
		if(*lastTimeMOSFETControl){
			*lastTimeMOSFET = millis();
			*lastTimeMOSFETControl = 0;
		}
		
		if(millis() > *lastTimeMOSFET + 2000){
			*controlMOSFET = 0;
		}
	}
	
	
	if(strcmp(statu, "Kurtarma") == 0){
		
		digitalWrite(10, HIGH);
		
		//İnişten 1 dakika sonra sadece buzzer çalıştır.
		if(*lastTimeBUZZERControl){
			*lastTimeBUZZER = millis();
			*lastTimeBUZZERControl = 0;
		}
		
		if(millis() > *lastTimeBUZZER + 3000){
			
			system("sudo killall verticalStabilization yawStabilization servod");
			
		}
		
		if(millis() > *lastTimeBUZZER + 60000){
			
			system("sudo /home/pi/Desktop/TURKSAT/buZZers");
			system("sudo killall servod bno055 gps ina219 ms5611 raspivid verticalStabilization yawStabilization flightSoftware");
			
		}
	}
	else{
		digitalWrite(10, LOW);
	}
	
	
}


void stopCoaxiel(){
	
	system("echo P1-11=100 > /dev/servoblaster");
	system("echo P1-13=100 > /dev/servoblaster");
	
	system("sudo killall verticalStabilization yawStabilization servod");
}
