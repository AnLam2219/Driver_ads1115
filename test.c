#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#define ADS_ADDR 0x48 
int main (void){
	int fd;
	fd = wiringPiI2CSetup(ADS_ADDR);
	/* config */
	//set address pointer 0x01 to write
	wiringPiI2CWrite(fd,0x01);
	int init = 0x4382;
	// init = init|(1<<8);
	wiringPiI2CWriteReg16(fd,0x01,init);
	//set address pointer 0x00 to read
	wiringPiI2CWrite(fd, 0x00);
	usleep(9000);
	int config = wiringPiI2CReadReg16(fd,0x01);
	printf("Config %x \n",config);
	int Lo_thresh_raw = wiringPiI2CReadReg16(fd,0x02);
	int Hi_thresh_raw = wiringPiI2CReadReg16(fd,0x03);
	int Lo_thresh = ((Lo_thresh_raw & 0xff) << 8)|((Lo_thresh_raw >> 8) & 0xff);
	int Hi_thresh = ((Hi_thresh_raw & 0xff) << 8)|((Hi_thresh_raw >> 8)  & 0xff);
	printf("Hi_thresh: %x\t",Hi_thresh);
	printf("Lo_thresh: %x\n",Lo_thresh);
	usleep(100000);
	while(1){
	uint32_t rawData = wiringPiI2CReadReg16(fd, 0x00);
	usleep(100000);
	uint32_t swappedData = ((rawData & 0xFF) << 8) | (rawData >> 8);
    float output = (swappedData)/32768.0*4.096;
	printf("ADC Value: %.4f\n", output);
	// printf("swappedData: %x\n", swappedData);
	} 
	return 0;
}