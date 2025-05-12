#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
 
#define DRIVER_NAME "ads1115_driver"
#define CLASS_NAME  "ads"

#define ADS1115_ADDR 	0x48
#define CONV_REG 		0x00
#define CONFIG_REG 		0x01
#define LoTHESH_REG		0x02
#define HiTHESH_REG 	0x03

static struct i2c_client *ads1115_client;
static int major_number;
static struct class *ads1115_class = NULL;
static struct device *ads1115_device = NULL;

//biến giao tiếp toàn cục giữa write-read
static int config[3];

//character device functions
static int ads1115_open(struct inode *inode, struct file *file) {
    return 0;
}

static int ads1115_release(struct inode *inode, struct file *file) {
    return 0;
}

static int ads1115_write_register(struct i2c_client *client,u8 reg,u16 value){
	// Gửi 3 byte đến thiết bị: [0x01][MSB][LSB] để cấu hình ADS1115
    int ret = i2c_smbus_write_word_data(client, reg, cpu_to_be16(value));
    if (ret < 0) {
        printk(KERN_ERR "ads1115: Failed to write to reg=0x%02x val=0x%04x\n", reg, value);
        return -EIO;
    }
	return 0;
}
static ssize_t ads1115_write(struct file *file, const char __user *buf, size_t len, loff_t *offset)
{
	
	char kbuf[32];  // Buffer trong kernel space
	ssize_t ret;
	u8 reg;
    u16 value;
	// Kiểm tra độ dài
    if (len > sizeof(kbuf)) {
        printk(KERN_ERR "Data size too large\n");
        return -EINVAL;  // Trả về lỗi nếu dữ liệu quá lớn
    }    
	// Kiểm tra đúng 3 byte dữ liệu để ghi cấu hình
    if (len != 3) {
        printk(KERN_ERR "ads1115: Expected 3 bytes: register + 2 config bytes\n");
        return -EINVAL;
    }
	// Copy dữ liệu từ user space vào kernel space
	ret = copy_from_user(kbuf, buf, len);
    if (ret) {
        printk(KERN_ERR "Failed to copy data from user\n");
        return -EFAULT;  // Trả về lỗi nếu copy thất bại
    }
	//copy thành biến toàn cục 
	config[0] = kbuf[0];
	config[1] = kbuf[1];
	config[2] = kbuf[2];
	reg = kbuf[0];
    value = (kbuf[1] << 8) | kbuf[2];  // Ghép MSB và LSB thành 16-bit value
	
	// xử lý cấu hình thanh ghi
	printk(KERN_INFO "ads1115: Configuration written: reg=0x%02x val=0x%04x\n", reg, value);
	ads1115_write_register(ads1115_client,reg,value);
	return len;
}

static ssize_t ads1115_read(struct file *file, char __user *buf, size_t len, loff_t *offset)
{
    char rbuf[32];  // Dữ liệu cần trả về từ kernel
    ssize_t ret;
	u8 reg_c;
	u16 value;
	u8 data[2];
	s16 adc_value;
	u16 check_reg;
	u8 reg = CONV_REG;;  // Conversion register
	
	if ((config[1] & (1<<0)) == 0){ //continue mode
			// Gửi địa chỉ thanh ghi Conversion (0x00)
		ret = i2c_master_send(ads1115_client, &reg, 1);
		if (ret < 0) {
			printk(KERN_ERR "Failed to set pointer to conversion register\n");
			return -EIO;
		}
		// Đọc 2 byte từ Conversion register
		ret = i2c_smbus_read_i2c_block_data(ads1115_client, reg, 2,data);
		if (ret < 0) {
			printk(KERN_ERR "Failed to read conversion data\n");
			return -EIO;
		}
		printk(KERN_INFO "Continous mode\n");
	}
	else {
		//bật lại bit OS
		reg_c = config[0];
		value = (config[1] << 8) | config[2];
		ads1115_write_register(ads1115_client,reg_c,value);
		
		ret = i2c_master_send(ads1115_client, &reg, 1);
		if (ret < 0) {
			printk(KERN_ERR "Failed to set pointer to conversion register\n");
			return -EIO;
		}
		// Đọc 2 byte từ Conversion register
		ret = i2c_smbus_read_i2c_block_data(ads1115_client, reg, 2,data);
		if (ret < 0) {
			printk(KERN_ERR "Failed to read conversion data\n");
			return -EIO;
		}
		printk(KERN_INFO "Single mode\n");
	}
	
	adc_value = (data[0] << 8) | data[1];
	memcpy(rbuf, &adc_value, sizeof(adc_value));
    // Nếu user không yêu cầu đọc gì, trả về 0
    if (*offset == 0) {
        ret = copy_to_user(buf, rbuf, strlen(rbuf) + 1);  // +1 để bao gồm ký tự NULL kết thúc chuỗi
        if (ret) {
            printk(KERN_ERR "Failed to copy data to user\n");
            return -EFAULT;  // Trả về lỗi nếu copy thất bại
        }

        *offset += strlen(rbuf) + 1;  // Cập nhật offset
        return strlen(rbuf) + 1;  // Trả về số byte đã đọc
    }
	
	
	ret = i2c_smbus_read_i2c_block_data(ads1115_client, CONFIG_REG, 2,data);
	if (ret < 0) {
		printk(KERN_ERR "Failed to read conversion data\n");
		return -EIO;
	}
	check_reg = (data[0] << 8) | data[1];
	printk(KERN_INFO "ads1115: Configuration written: reg=0x%02x val=0x%04x\n", CONFIG_REG, check_reg);
    
	return 0;  // Nếu không có dữ liệu để đọc nữa
}


//i2c device functions
static int ads1115_probe(struct i2c_client *client, const struct i2c_device_id *id) {
	printk(KERN_INFO "ADS1115 at address 0x%02x on bus %d\n", client->addr, client->adapter->nr);
	ads1115_client = client;
	//cấu hình đầu tiên cho ads1115 ở dây
	if (!ads1115_client) {
        dev_err(&client->dev, "ADS1115 client is NULL\n");
        return -ENODEV;  // Trả về lỗi nếu client bị null
    }
    printk(KERN_INFO "ADS1115 driver installed\n");
    return 0;
}
static void ads1115_remove(struct i2c_client *client)
{
    printk(KERN_INFO "ADS1115 driver removed\n");

    // Clean up
    
}
//TÌm và đăng kí id cho device
static const struct i2c_device_id ads1115_id[] = {
    { "ads1115", 0 }, //đăng kí id trống 
    { }
};
MODULE_DEVICE_TABLE(i2c, ads1115_id);

//biến điều khiển cho character device 
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = ads1115_open,
    .read = ads1115_read,
    .write = ads1115_write,
    .release = ads1115_release,
};

//biến khởi tạo khi có thấy địa chỉ tương ứng
static struct i2c_driver ads1115_driver = {
    .driver = {
        .name = DRIVER_NAME,				// tên driver (phải khớp với device tree)
		.owner  = THIS_MODULE,		
        // .of_match_table = my_i2c_of_match, 	// cho Device Tree
    },
    .probe = ads1115_probe,
    .remove = ads1115_remove,
    .id_table = ads1115_id,           		// cho non-DT hệ thống
};



static int __init ads1115_init(void)
{	
    printk(KERN_INFO "Initializing ADS1115 driver\n");
	// Đăng ký character device
    major_number = register_chrdev(0, DRIVER_NAME, &fops);
    if (major_number < 0) {
        printk(KERN_ERR "Failed to register char device\n");
        return major_number;
    }
	// Tạo class
	ads1115_class = class_create(THIS_MODULE, DRIVER_NAME);
	if (IS_ERR(ads1115_class)) {
        unregister_chrdev(major_number, DRIVER_NAME);
        printk(KERN_ALERT "Failed to register device class\n");
        return PTR_ERR(ads1115_class);
    }
	// Tạo device trong /dev
	ads1115_device = device_create(ads1115_class, NULL, MKDEV(major_number, 0), NULL, DRIVER_NAME);
	if (IS_ERR(ads1115_device)) {
        class_destroy(ads1115_class);
        unregister_chrdev(major_number, DRIVER_NAME);
        printk(KERN_ALERT "Failed to create the device\n");
        return PTR_ERR(ads1115_device);
    }
	printk(KERN_INFO "ADS1115 device registered correctly\n");
    return i2c_add_driver(&ads1115_driver);  //register driver in kernel but it run on i2c base
}

static void __exit ads1115_exit(void)
{
    printk(KERN_INFO "Exiting ADS1115 driver\n");
	i2c_del_driver(&ads1115_driver);
	
	device_destroy(ads1115_class, MKDEV(major_number, 0));
    class_unregister(ads1115_class);
    class_destroy(ads1115_class);
    unregister_chrdev(major_number, DRIVER_NAME);
    printk(KERN_INFO "ADS1115 device unregistered\n");
    
}

module_init(ads1115_init);
module_exit(ads1115_exit);

MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("ADS1115 I2C Client Driver");
MODULE_LICENSE("GPL");