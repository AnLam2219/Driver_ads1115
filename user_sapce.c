#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>      // open()
#include <unistd.h>     // read(), write(), close()
#include <stdint.h>     // uint8_t, int16_t
#include <string.h>     // memset()

int main() {
	const char *device = "/dev/ads1115_driver";
    int fd = open(device, O_RDWR);  // Mở thiết bị để đọc và ghi
    if (fd < 0) {
        perror("Failed to open /dev/ads1115_driver");
        return EXIT_FAILURE;
    }
	// ---------- Gửi cấu hình tới kernel (ví dụ cấu hình ADC mode) ----------
    uint8_t config_bytes[3] = { 0x01, 0x84, 0x63 };  // Dữ liệu nhị phân

    ssize_t wret = write(fd, config_bytes, sizeof(config_bytes));
    if (wret < 0) {
        perror("Failed to write to /dev/ads1115_driver");
        close(fd);
        return EXIT_FAILURE;
    }
	printf("wret: %d\n",wret);
    // ---------- Đọc giá trị ADC từ kernel ----------
    int16_t adc_value = 0;
    ssize_t rret = read(fd, &adc_value, sizeof(adc_value));
    if (rret < 0) {
        perror("Failed to read from /dev/ads1115");
        close(fd);
        return EXIT_FAILURE;
    }
	printf("Raw ADC Value: %d\n", adc_value);
	// Chuyển sang điện áp (giả định PGA ±4.096V → 1LSB = 125uV)
    float voltage = (adc_value * 4.096) / 32768.0;
    printf("Voltage: %.4f V\n", voltage);
	close(fd);
	return 0;
}