//sudo gcc ina219.c raspberrypi4b_driver_ina219_interface.c iic.c driver_ina219_basic.c driver_ina219.c -lm -lwiringPi -o ina219

#include "driver_ina219_basic.h"
#include "iic.h"
#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>

int main(){
	
    
    float mA;
    
    ina219_basic_init(INA219_ADDRESS_0, 0.1);
                      
                        
    while(1){
                            
      ina219_basic_read(&mA);
                            
                            
      FILE *ina219WriteDosya = fopen("/home/pi/Desktop/TURKSAT/ina219/INA219datas.txt", "w");  // w => Üzerine yaz	
      fprintf(ina219WriteDosya, "%f", mA);
      fclose(ina219WriteDosya);
                            
      delay(900);
    }
                        
    ina219_basic_deinit();

    return 0;
}
