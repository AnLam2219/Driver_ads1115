#include <stdio.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <unistd.h>
#include <math.h>

#define ADDRESS 0x48
#define CONVERT 0x00
#define CONFIG 0X01
uint16_t ADS1115_init(char PIN_P,char PIN_N, float PGA, char MODE, uint16_t DATA_RATE)
{
    uint16_t Config =0;

    uint8_t pin=0;
    if(PIN_P == '0' && PIN_N == '1') pin=0;
    else if(PIN_P == '0' && PIN_N == '3') pin=1;
    else if(PIN_P == '1' && PIN_N == '3') pin=2;
    else if(PIN_P == '2' && PIN_N == '3') pin=3;
    else if(PIN_P == '0' && PIN_N == 'G') pin=4;
    else if(PIN_P == '1' && PIN_N == 'G') pin=5;
    else if(PIN_P == '2' && PIN_N == 'G') pin=6;
    else if(PIN_P == '3' && PIN_N == 'G') pin=7;

    uint8_t pga=0;
    if(PGA > 6) pga =0;
    else if(PGA > 4) pga =1;
    else if(PGA > 2) pga =2;
    else if(PGA > 1) pga =3;
    else if(PGA > 0.5) pga =4;
    else pga = 5;

    uint8_t mode=1;
    if(MODE == 'C') mode =0; 
    else mode =1;

    uint8_t data_rate=0b0100;
    if(DATA_RATE==860) data_rate=0b111;
    else if(DATA_RATE==475) data_rate=0b110;
    else if(DATA_RATE==250) data_rate=0b101;
    else data_rate = log2(DATA_RATE/8);

    Config |= (1<<15) | (pin<<12) | (pga<<9) | (mode << 8) | (data_rate << 5) ;
    //|(0b11 <<0);
    return Config;
}
int main()
{
    int ads = wiringPiI2CSetup(ADDRESS);
    uint16_t config =0;
    /*config |= (0b100 << 12); // MUX[2:0] = AIN0 vs GND
    config |= (0b000 << 9);  // PGA ± 6.144
    config |= (0b0 << 8);    // MODE = Continuous
    config |= (0b100 << 5);  // Data Rate = 128 SPS
    config |= 0x03;          // Comparator off  */
    //config = 0b1100000010000011;
    config = ADS1115_init('0','G',6.144,'C',128);
    //printf("\t%d\n",config);

    uint16_t config_s = (config<<8) | (config>>8);
    wiringPiI2CWriteReg16(ads,CONFIG,config_s);
    wiringPiI2CWriteReg16(ads,0x03,0x803E); //set giới hạn trên là 3v/6.144
    wiringPiI2CWriteReg16(ads,0x02,0xB414); //set giới hạn dưới là 1v/6.144
    uint16_t config_r = wiringPiI2CReadReg16(ads,CONFIG);
    uint16_t test = (config_r<<8)|(config_r>>8);
    printf("%x\n",test);
    usleep(8000); 
    float FS;
    uint16_t v_ss = (test& 0xE00)>>9; //tách 3 bit 9 10 11
    printf("%d\n",v_ss);
    if(v_ss == 0) FS =6.144;
    else if(v_ss ==1) FS =4.096; 
    else if(v_ss ==2) FS =2.048;
    else if(v_ss ==3) FS = 1.024;
    else if(v_ss ==4) FS =0.512;
    else FS =0.256;
    while (1)
    {
        uint16_t data_r = wiringPiI2CReadReg16(ads,CONVERT);
        uint16_t data = (data_r<<8)|(data_r>>8);
        float V = data*FS/32768.0;
        printf("ADC: %d | V = %.2f V\n", data, V);
        delay(100);
    }
    
}