#include <wiringPi.h>

int main(){
	
	wiringPiSetup();
	pinMode(10, OUTPUT);
	
	while(1){
		digitalWrite(10, HIGH);
	}
	
	return 0;
}
