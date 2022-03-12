# Flight Software of TURKSAT 10A

BNO055datas.txt,
GCS.txt,
GPSdatas.txt,
INA219datas.txt,
MS5611datas.txt,
OOPROM.txt,
motorPwm.txt,
statuDatas.txt,
telemetry.txt files are the files that the data is saved.

rpiservert.py,
pythonserver.py,
pythonclient.py,
commands.py are the scripts that communication with the ground station is provided.

flightSoftware.c is the main flight software.

preflightInseption.c is the script that makes the necessary arrangements on the raspberry pi before the flight.

verticalStabilization.c,
yawStabilization.c,
descentVelocity.c are the scripts that keeps the model satellite stable on pitch, roll and yaw axes and determines the landing speed.

bno055.c,
gps.c,
ina219sensor.c,
ms5611.c are the scripts that reads sensor data.

