//Compile => sudo gcc preflightInseption.c -o preflightInseption

#include <stdlib.h>

int main(){
	
	system("sudo killall flightSoftware servod buZZers");
	system("sudo truncate -s 0 telemetry.txt");
	system("sudo truncate -s 0 OOPROM.txt");
	
	
	return 0;
}
