#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>      // open()
#include <unistd.h>     // read(), write(), close()
#include <stdint.h>     // uint8_t, int16_t
#include <string.h>     // memset()
#include <math.h>


uint16_t ADS1115_init(char PIN_P,char PIN_N, float PGA, char MODE, uint16_t DATA_RATE, char MODE_COMPARATOR, uint8_t NUMBER_CONVERSION)
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

    uint8_t mode_comparator =0;
    if(MODE_COMPARATOR == 'W') mode_comparator =1; 

    uint8_t number = NUMBER_CONVERSION;

    Config |= (1<<15) | (pin<<12) | (pga<<9) | (mode << 8) | (data_rate << 5) | (mode_comparator <<4) |(number <<0);
    printf("%d\n",Config);
    return Config;
}

int main() {
	const char *device = "/dev/ads1115_driver";
    int fd = open(device, O_RDWR);  // Mở thiết bị để đọc và ghi
    if (fd < 0) {
        perror("Failed to open /dev/ads1115_driver");
        return EXIT_FAILURE;
    }
	// ---------- Gửi cấu hình tới kernel (ví dụ cấu hình ADC mode) ----------
    int16_t config = ADS1115_init('0','G',4.096,'C',128,'W', 0);
	int MSB = (config >>8) & 0xff;
	int LSB = config & 0xff;
	uint8_t config_bytes[3] = { 0x01, MSB ,LSB };  // Dữ liệu nhị phân
    ssize_t wret = write(fd, config_bytes, sizeof(config_bytes));
    if (wret < 0) {
        perror("Failed to write to /dev/ads1115_driver");
        close(fd);
        return EXIT_FAILURE;
    }
	printf("wret: %d\n",wret);
	usleep(100000);
	//-------------thêm các giá trị ngưỡng cảnh báo-------------
	/*Ngưỡng dưới Lo_Thrserh*/
	int Lo_thresh_MSB = 0x20;
	int Lo_thresh_LSB = 0x00;
	config_bytes[0] = 0x02;
	config_bytes[1] = Lo_thresh_MSB;
	config_bytes[2] = Lo_thresh_LSB;
	wret = write(fd, config_bytes, sizeof(config_bytes));
    if (wret < 0) {
        perror("Failed to write to /dev/ads1115_driver");
        close(fd);
        return EXIT_FAILURE;
    }
	/*Ngưỡng trên Hi_Thresh*/
	int Hi_thresh_MSB = 0x7f;
	int Hi_thresh_LSB = 0x00;
	config_bytes[0] = 0x03;
	config_bytes[1] = Hi_thresh_MSB;
	config_bytes[2] = Hi_thresh_LSB;
	wret = write(fd, config_bytes, sizeof(config_bytes));
    if (wret < 0) {
        perror("Failed to write to /dev/ads1115_driver");
        close(fd);
        return EXIT_FAILURE;
    }
    // ---------- Đọc giá trị ADC từ kernel ----------
    int32_t adc_value = 0;
    ssize_t rret = read(fd, &adc_value, sizeof(adc_value));
    if (rret < 0) {
        perror("Failed to read from /dev/ads1115");
        close(fd);
        return EXIT_FAILURE;
    }
	printf("Raw ADC Value: %d\n", adc_value);
	uint16_t test = config & 0xff|(config >>8) & 0xff;
    printf("%x\n",test);
    usleep(8000);
	// Chuyển sang điện áp theo code mẫu sau hoặc có thể truyền theo datasheet
	float FSR;
    uint16_t v_ss = (test& 0xE00)>>9; //tách 3 bit 9 10 11
    printf("%d\n",v_ss);
    if(v_ss == 0) FSR =6.144;
    else if(v_ss ==1) FSR =4.096; 
    else if(v_ss ==2) FSR =2.048;
    else if(v_ss ==3) FSR = 1.024;
    else if(v_ss ==4) FSR =0.512;
    else FSR =0.256;
    float voltage = adc_value*1.0/32768.0*FSR;
    printf("Voltage: %.4f V\n", voltage);
	
	close(fd);
	return 0;
}
