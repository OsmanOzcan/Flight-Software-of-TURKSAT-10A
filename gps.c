// Compile => sudo gcc gps.c -lgps -lm -o gps

#include <stdio.h>
#include <stdlib.h>
#include <gps.h>


int main() {
    // Open
    gps_init();
    loc_t data;
    
    float latitude = 0, longitude = 0, altitude = 0;

    while (1) {
	
        gps_location(&data);
	
	latitude = data.latitude;
	longitude = data.longitude;
	altitude = data.altitude;
	
	FILE *gpsWriteDosya = fopen("/home/pi/Desktop/TURKSAT/GPSdatas.txt", "w");  // w => Üzerine yaz	
	fprintf(gpsWriteDosya, "%f %f %f", latitude, longitude, altitude);
	fclose(gpsWriteDosya);
		
    }

    return 0;
}
