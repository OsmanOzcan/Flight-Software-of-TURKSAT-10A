#include <wiringPi.h>

int main(){
	
	
	wiringPiSetup();
	
	pinMode(10, OUTPUT);
	pinMode(24, OUTPUT);
	pinMode(25, OUTPUT);
	
	digitalWrite(10, LOW);
	digitalWrite(24, LOW);
	digitalWrite(25, LOW);
	
	return 0;
}
