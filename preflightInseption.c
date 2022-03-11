#include <stdlib.h>

int main(){
	
	system("sudo killall servod flightSoftware buZZers bno055 gps ina219 ms5611 raspivid verticalStabilization yawStabilization descentVelocity");
	system("sudo truncate -s 0 telemetry.txt");
	system("sudo truncate -s 0 OOPROM.txt");
	system("sudo truncate -s 0 GCS.txt");
	system("sudo truncate -s 0 GPSdatas.txt");
	system("sudo truncate -s 0 BNO055datas.txt");
	system("sudo truncate -s 0 INA219datas.txt");
	system("sudo truncate -s 0 MS5611datas.txt");
	system("sudo truncate -s 0 statuDatas.txt");
	system("sudo truncate -s 0 motorPwm.txt");
	system("sudo truncate -s 0 ina219/INA219datas.txt");
	system("sudo rm /home/pi/video.mp4");
	system("sudo rm /home/pi/canligoruntu.h264");
	
	
	return 0;
}
