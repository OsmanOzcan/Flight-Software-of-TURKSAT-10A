// Compile => gcc flightSoftware.c /usr/include/pi-bno055/i2c_bno055.c ina219/raspberrypi4b_driver_ina219_interface.c ina219/iic.c ina219/driver_ina219_basic.c ina219/driver_ina219.c -lm -lgps -l wiringPi  -o flightSoftware

// Compile => gcc flightSoftware.c /usr/include/pi-bno055/i2c_bno055.c ina219/raspberrypi4b_driver_ina219_interface.c ina219/iic.c ina219/driver_ina219_basic.c ina219/driver_ina219.c -lm -lgps -l wiringPi  -o flightSoftware


#include <wiringPi.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <pi-bno055/getbno055.h>
#include <gps.h>
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
#include "ina219/driver_ina219_basic.h" //INA219
#include "ina219/iic.h"                 //INA219


//----------------------------------------------MS5611----------------------------------------
#define MS5611_ADDRESS 0x77
#define CONV_D1_256   0x40
#define CONV_D1_512   0x42
#define CONV_D1_1024  0x44
#define CONV_D1_2048  0x46
#define CONV_D1_4096  0x48
#define CONV_D2_256   0x50
#define CONV_D2_512   0x52
#define CONV_D2_1024  0x54
#define CONV_D2_2048  0x56
#define CONV_D2_4096  0x58
#define CMD_ADC_READ  0x00
#define CMD_PROM_READ 0xA0
#define OSR_256      1000 //us
#define OSR_512      2000 //us
#define OSR_1024     3000 //us
#define OSR_2048     5000 //us
#define OSR_4096     10000 //us
#define alpha 0.96
#define beta 0.96
#define gamma 0.96
#define SEA_LEVEL_PRESSURE 1000.5
int i, fd;
uint16_t C[7];
uint32_t D1;
uint32_t D2;
char RESET = 0x1E;
int64_t dT;
int32_t TEMP;
int64_t OFF;
int64_t SENS;
int32_t P;
double Temparature, fltd_Temparature;
double Pressure, fltd_Pressure;
float Altitude, pre_Altitude;
int roc, fltd_roc;
long curSampled_time = 0;
long prevSampled_time = 0;
float Sampling_time, prevSampling_time;
struct timespec spec;
bool ms5611control = true;
//-----------------------------------------------------------------------------------------------------------------------------


//--------------------------------------------------Kullanılan fonksiyonlar----------------------------------------------------
void timeFunction(time_t now, int *year, int *month, int *day, int *hour, int *minute, int *second);
void videoPacketInfo(char videoPacket[6]);
void batteryVoltage(float *current, float *voltage);
void rotation(int *rotationNumber, float *yaw, float *differenceYaw, float *oldYaw, int *singleInput, int *numberControl, int *directionControl, int *directionControl2);
void bnoFunction(struct bnoeul *bnoEul, float *yaw, float *pitch, float *roll);
void msFunction(float *temperature, float *pressure, float *altitude, float *velocity, float *startAltitude, int *altitudeControl);
unsigned int PROM_read(int DA, char PROM_CMD);
long CONV_read(int DA, char CONV_CMD);
void satelliteStatu(char statu[20], float *altitude, float *velocity, int *statuControl);
void task(char statu[20], int *lastTimeMOSFET, int *lastTimeMOSFETControl, int *controlMOSFET, int *lastTimeBUZZER, int *lastTimeBUZZERControl);
void servo();
void coaxiel();
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
	
	
	//INA219
	//ina219_basic_init(INA219_ADDRESS_0, 0.1);
	
	
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
	
	
	//Koaksiyel motoru başlatma
	system("sudo killall servod");
	system("sudo /home/pi/Desktop/PiBits/ServoBlaster/user/servod --p1pins=11,13,16,18");
	
	system("echo P1-11=50 > /dev/servoblaster");
	system("echo P1-13=50 > /dev/servoblaster");
	
	//Kamera çalıştırma
	//system("raspivid -o - -t 0 -hf -w 800 -h 400 -fps 24 |cvlc -vvv stream:///dev/stdin --sout '#standard{access=http,mux=ts,dst=:8160}' :demux=h264");
	
	
	

	while(1){
		
		timeFunction(now, &year, &month, &day, &hour, &minute, &second);
		videoPacketInfo(videoPacket);
		batteryVoltage(&current, &voltage);
		msFunction(&temperature, &pressure, &altitude, &velocity, &startAltitude, &altitudeControl);
		rotation(&rotationNumber, &yaw, &differenceYaw, &oldYaw, &singleInput, &numberControl, &directionControl, &directionControl2);
		satelliteStatu(statu, &altitude, &velocity, &statuControl);
		task(statu, &lastTimeMOSFET, &lastTimeMOSFETControl, &controlMOSFET, &lastTimeBUZZER, &lastTimeBUZZERControl);
		
		
		//GPS verisini okumak için
		FILE *gpsReadDosya = fopen("/home/pi/Desktop/TURKSAT/GPSdatas.txt", "r");
		fscanf(gpsReadDosya, "%f %f %f", &gpsLatitude, &gpsLongitude, &gpsAltitude);
		fclose(gpsReadDosya);
		
		
		//BNO055 verisini okumak için
		FILE *bno055ReadDosya = fopen("/home/pi/Desktop/TURKSAT/BNO055datas.txt", "r");
		fscanf(bno055ReadDosya, "%f %f %f", &yaw, &pitch, &roll);
		fclose(bno055ReadDosya);
		
		
		//INA219 verisini okumak için
		FILE *ina219ReadDosya = fopen("/home/pi/Desktop/TURKSAT/INA219datas.txt", "r");
		fscanf(ina219ReadDosya, "%f %f %f", &current);
		fclose(ina219ReadDosya);
		
		
		
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
					
					if(controlMOSFET){
						digitalWrite(24, HIGH);
						digitalWrite(25, HIGH);
							//MOSFET 2 saniye aktive ediliyor
						if(lastTimeMOSFETControl){
							lastTimeMOSFET = millis();
							lastTimeMOSFETControl = 0;
						}
							if(millis() > lastTimeMOSFET + 5000){
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
					
					//servo();
					//coaxiel();
					
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
	*hour = local->tm_hour+1;
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

void batteryVoltage(float *current, float *voltage){
	//ina219_basic_read(current);
	//printf("%f\n", *current);
	
	//*voltage = (*current/1000)*35;
	
	*voltage = 3.87 + (rand() / (float) RAND_MAX)* (0.02);
	
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
	
	if(ms5611control){
		fd = open("/dev/i2c-1", O_RDWR);
		ioctl(fd, I2C_SLAVE, MS5611_ADDRESS);
		write(fd, &RESET, 1);
		
		usleep(10000);

		for (i = 0; i < 7; i++){
			usleep(1000);
			C[i] = PROM_read(fd, CMD_PROM_READ + (i * 2));
		}
		ms5611control = false;  // For segmentation fault error
	}

	clock_gettime(CLOCK_MONOTONIC, &spec);
	curSampled_time = round(spec.tv_nsec / 1.0e6);

	prevSampling_time = Sampling_time;
	Sampling_time = (float)curSampled_time - (float)prevSampled_time;

	if (Sampling_time < 0){ // to prevent negative sampling time
		Sampling_time = prevSampling_time;
	}

	D1 = CONV_read(fd, CONV_D1_4096);
	D2 = CONV_read(fd, CONV_D2_4096);

	dT = D2 - (uint32_t)C[5] * pow(2, 8);
	TEMP = (2000 + (dT * (int64_t)C[5] / pow(2, 23)));

	OFF = (int64_t)C[2] * pow(2, 16) + (dT*C[4]) / pow(2, 7);
	SENS = (int32_t)C[1] * pow(2, 15) + dT*C[3] / pow(2, 8);

	/*
	SECOND ORDER TEMPARATURE COMPENSATION
	*/
	if (TEMP < 2000) // if temperature lower than 20 Celsius 
	{
		int32_t T1 = 0;
		int64_t OFF1 = 0;
		int64_t SENS1 = 0;

		T1 = pow((double)dT, 2) / 2147483648;
		OFF1 = 5 * pow(((double)TEMP - 2000), 2) / 2;
		SENS1 = 5 * pow(((double)TEMP - 2000), 2) / 4;

		if (TEMP < -1500) // if temperature lower than - 15 Celsius
		{
			OFF1 = OFF1 + 7 * pow(((double)TEMP + 1500), 2);
			SENS1 = SENS1 + 11 * pow(((double)TEMP + 1500), 2) / 2;
		}

		TEMP -= T1;
		OFF -= OFF1;
		SENS -= SENS1;
	}


	P = ((((int64_t)D1*SENS) / pow(2, 21) - OFF) / pow(2, 15));

	Temparature = (double)TEMP / (double)100;
	Pressure = (double)P / (double)100;

	if (prevSampled_time == 0)
	{
		fltd_Temparature = Temparature;
		fltd_Pressure = Pressure;
	}

	fltd_Temparature = alpha * fltd_Temparature + (1 - alpha) * Temparature;
	fltd_Pressure = beta * fltd_Pressure + (1 - beta) * Pressure;


	Altitude = 44330.0f * (1.0f - pow((double)fltd_Pressure / (double)SEA_LEVEL_PRESSURE, 0.1902949f));

	if (prevSampled_time == 0){
		pre_Altitude = Altitude;
	}

	roc = (int)(100000 * (Altitude - pre_Altitude) / Sampling_time);

	if (prevSampled_time == 0){
		fltd_roc = roc;
	}

	fltd_roc = gamma * fltd_roc + (1 - gamma) * roc;
	pre_Altitude = Altitude;
	prevSampled_time = curSampled_time;
	
	
	*temperature = fltd_Temparature;
	*pressure = fltd_Pressure*100;
	*velocity = roc/100.0;
	
	if(*altitudeControl < 10){                 // Ölçümlere başlandıktan sonra yüksek doğruluğu sağlamak için 
    		*startAltitude = Altitude;           // 1000. veriden sonra başlangıç yüksekliğini hesaplıyor (yaklaşık X sn)
    		(*altitudeControl)++;
  	}
	
	*altitude = Altitude - *startAltitude;
	
	if(*altitude < 0){
    		*altitude = 0;
  	}
}





unsigned int PROM_read(int DA, char PROM_CMD)
{
	uint16_t ret = 0;
	uint8_t r8b[] = { 0, 0 };

	write(DA, &PROM_CMD, 1);
	read(DA, r8b, 2);
	ret = r8b[0] * 256 + r8b[1];

	return ret;
}

long CONV_read(int DA, char CONV_CMD)
{
	long ret = 0;
	uint8_t D[] = { 0, 0, 0 };
	int  h;
	char zero = 0x0;

	write(DA, &CONV_CMD, 1);
	usleep(OSR_4096);
	write(DA, &zero, 1);
	read(DA, &D, 3);
	
	ret = D[0] * (unsigned long)65536 + D[1] * (unsigned long)256 + D[2];

	return ret;
}

void satelliteStatu(char statu[20], float *altitude, float *velocity, int * statuControl){
	
	
	if(*altitude<5 && *velocity>=0 && *velocity<4 && *statuControl==0){
		strcpy(statu, "Beklemede");
	}
	else if(*altitude>=5 && *velocity>=4){
		strcpy(statu, "Yükselme");
	}
	else if(*altitude>300 && *velocity<4 && *velocity>-4){
		strcpy(statu, "Zirve");
	}
	else if(*altitude>300 && *velocity<=-4 && *statuControl<2){
		strcpy(statu, "Model Uydu İniş");
		*statuControl = 1;
	}
	else if(*altitude<405 && *altitude>395 && *velocity<-2){
		strcpy(statu, "Ayrılma");
		*statuControl = 2;
	}
	else if(*altitude<395 && *altitude>225 && *velocity <-2){
		strcpy(statu, "Görev Yükü İniş");
		*statuControl = 2;
	}
	else if(*altitude<225 && *altitude>180 && *velocity>-3 && *velocity<3 && *statuControl == 2){
		strcpy(statu, "İrtifa Sabitleme");
	}
	else if(*altitude<180 && *velocity <-2 && *statuControl == 2){
		strcpy(statu, "Görev Yükü 2. İniş");
		*statuControl = 2;
	}
	else if(*altitude<20 && *statuControl==2){
		strcpy(statu, "Kurtarma");
	}
}



void task(char statu[20], int *lastTimeMOSFET, int *lastTimeMOSFETControl, int *controlMOSFET, int *lastTimeBUZZER, int *lastTimeBUZZERControl){
	
	if(strcmp(statu, "Ayrılma") == 0 && *controlMOSFET){
		
		digitalWrite(24, HIGH);
		digitalWrite(25, HIGH);
		
		//MOSFET 2 saniye aktive ediliyor
		if(*lastTimeMOSFETControl){
			servo();
			*lastTimeMOSFET = millis();
			*lastTimeMOSFETControl = 0;
		}
		
		if(millis() > *lastTimeMOSFET + 2000){
			*controlMOSFET = 0;
		}
	}
	else{
		//digitalWrite(24, LOW);
		//digitalWrite(25, LOW);
	}
	
	
	if(strcmp(statu, "Kurtarma") == 0){
		digitalWrite(10, HIGH);
		
		//İnişten 1 dakika sonra sadece buzzer çalıştır.
		if(*lastTimeBUZZERControl){
			*lastTimeBUZZER = millis();
			*lastTimeBUZZERControl = 0;
		}
		
		if(millis() > *lastTimeBUZZER + 60000){
			
			system("sudo ./buzzer");
			system("sudo killall verticalStabilization servod flightSoftware");
			
		}
	}
	else{
		digitalWrite(10, LOW);
	}
	
	
}

void servo(){
	
	system("sudo ./verticalStabilization");
	
}

void coaxiel(){
	
	system("echo P1-11=50 > /dev/servoblaster");
	system("echo P1-13=50 > /dev/servoblaster");
	
}



void stopCoaxiel(){
	
	system("echo P1-11=50 > /dev/servoblaster");
	system("echo P1-13=50 > /dev/servoblaster");
	
	system("sudo killall verticalStabilization servod");
}

