// Compile => sudo gcc bno055.c /usr/include/pi-bno055/i2c_bno055.c -o bno055

#include <stdio.h>
#include <pi-bno055/getbno055.h>


int main() {
	
	get_i2cbus("/dev/i2c-1", "0x28");
	opmode_t newmode = ndof;
	struct bnoeul bnod;
	set_mode(newmode);
	
    
	while(1){
	
		get_eul(&bnod);
		//printf("EUL %3.4f %3.4f %3.4f\n", bnod.eul_head, bnod.eul_pitc, bnod.eul_roll);
		
		FILE *bno055WriteDosya = fopen("/home/pi/Desktop/TURKSAT/BNO055datas.txt", "w");  // w => Üzerine yaz	
		fprintf(bno055WriteDosya, "%f %f %f", bnod.eul_head, bnod.eul_pitc, bnod.eul_roll);
		fclose(bno055WriteDosya);
	
	}
	
	return 0;
}
