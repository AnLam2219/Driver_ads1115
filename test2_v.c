#include <stdio.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <unistd.h>
#include <stdint.h>
#define ADDRESS 0x48
#define CONVERT 0x00
#define CONFIG 0X01
/*uint16_t ADS1115_init(string PIN, float PGA, char MODE, uint16_t DATA_RATE)
{
    uint16_t Config =0;
    switch (PIN_P)
    {
    case 0:
        
        break;
    
    default:
        break;
    }

}*/
int main()
{
    int ads = wiringPiI2CSetup(ADDRESS);
    uint16_t config =0;
    /*config |= (0b100 << 12); // MUX[2:0] = AIN0 vs GND
    config |= (0b000 << 9);  // PGA ± 6.144
    config |= (0b0 << 8);    // MODE = Continuous
    config |= (0b100 << 5);  // Data Rate = 128 SPS
    config |= 0x03;          // Comparator off  */
    config = 0b1100000010000011;

    uint16_t config_s = (config<<8) | (config>>8);
    wiringPiI2CWriteReg16(ads,CONFIG,config_s);
    uint16_t config_r = wiringPiI2CReadReg16(ads,CONFIG);
    uint16_t test = (config_r<<8)|(config_r>>8);
    printf("%x\n",test);
    usleep(8000); 
    while (1)
    {
        uint16_t data_r = wiringPiI2CReadReg16(ads,CONVERT);
        uint16_t data = (data_r<<8)|(data_r>>8);
        float V = data*6.144/32768.0;
        printf("ADC: %d | V = %.2f V\n", data, V);
        delay(100);
    }
    
}