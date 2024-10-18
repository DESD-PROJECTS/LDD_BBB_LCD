#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/kernel.h>

#define I2C_BUS_AVAILABLE   (   2  )     //I2C bus
#define SLAVE_DEVICE_NAME   ("PCF8574")   //device name
#define LCD_SLAVE_ADDR      (  0x27 ) //LCD/I2C slave address

static struct i2c_adapter *lcd_i2c_adapter = NULL; //I2C adapter structure
static struct i2c_client  *lcd_i2c_client   = NULL; //I2C client
static void lcd_send_string(const char *str);
/*function to send data/command to the LCD*/
static int lcd_send(unsigned char data, unsigned char rs_flag)
{
    unsigned char buf[4];
    int ret;

   //initialize data with RS flag (RS=0 for command, RS=1 for data)
    unsigned char data_u = (data & 0xF0) | rs_flag | 0x08;         //backlight on (rs_flag | 0x08)
    unsigned char data_l = ((data << 4) & 0xF0) | rs_flag | 0x08; //backlight on

           //for high nibble
    buf[0] = data_u | 0x04; //enable bit high
    buf[1] = data_u;     //enable bit low
    ret = i2c_master_send(lcd_i2c_client, buf, 2);
    if (ret < 0) {
        pr_err("Error sending high nibble, return code: %d\n", ret);
        return ret;
    }

       //for low nibble
    buf[0] = data_l | 0x04;  //enable bit high
    buf[1] = data_l;     //enable bit low
    ret = i2c_master_send(lcd_i2c_client, buf, 2);
    if (ret < 0) {
        pr_err("Error sending low nibble, return code: %d\n", ret);
        return ret;
    }

 //wait for command to be processed by the LCD
    if(rs_flag == 0) {
        //for Command
        if(data == 0x01 || data == 0x02){ //clear display and return home require more than 1.52ms approx
            msleep(2);
        }else{
            //other commands require > 37µs as per HD44780
            usleep_range(50, 100);
        }
    }else{
         //for data mode
        usleep_range(50, 100);
    }

    return 0;
}

/*LCD initialization function*/
static int lcd_init(void)
{
    msleep(40);                       //wait for more than 40ms after VCC rises to 2.7V

                                   //initialization sequence
    lcd_send(0x30, 0);                   // function set
    msleep(5);                               //wait for more than 4.1ms

    lcd_send(0x30, 0);                  //wait for more than 100µs
    usleep_range(100, 200);                 

    lcd_send(0x30, 0);   
    usleep_range(100, 200);

    lcd_send(0x20, 0);         //set to 4-bit mode
    usleep_range(100, 200);

               
    lcd_send(0x28, 0);                  // 4-bit mode, 2 lines, 5x8 dots as per HD44780
    usleep_range(50, 100);

              //display on/off control:display on,cursor off,no blinking
    lcd_send(0x0C, 0);   //display on
    usleep_range(50, 100);

              
    lcd_send(0x06, 0);   //increment cursor,no display shift
    usleep_range(50, 100);

    
    lcd_send(0x01, 0);   //clear display
    msleep(2);           //wait for more than 1.52ms approx

    return 0;
}

//this function lcd_probe gets called when the slave has been found
static int lcd_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    int ret;

    pr_info("HD44780 LCD Probed\n");

    lcd_i2c_client = client;

    //initialize the LCD
    ret = lcd_init();
    if (ret < 0) {
        pr_err("LCD initialization failed\n");
        return ret;
    }
   lcd_send_string("Hello,BBB!");             //optionally, display a test message
    

    return 0;
}


static int lcd_remove(struct i2c_client *client) //function gets called when the slave is removed
{
    pr_info("HD44780 LCD Removed\n");
    return 0;
}

//I2C device ID structure
static const struct i2c_device_id lcd_id[] = {
    { SLAVE_DEVICE_NAME, 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c, lcd_id);

//I2C driver structure 
static struct i2c_driver lcd_driver = {
    .driver = {
        .name   = SLAVE_DEVICE_NAME,
        .owner  = THIS_MODULE,
    },
    .probe          = lcd_probe,
    .remove         = lcd_remove,
    .id_table       = lcd_id,
};

//I2C board info structure
static struct i2c_board_info lcd_i2c_board_info = {
    I2C_BOARD_INFO(SLAVE_DEVICE_NAME, LCD_SLAVE_ADDR)
};

static int __init lcd_driver_init(void)//module init function
{
    int ret;

    lcd_i2c_adapter = i2c_get_adapter(I2C_BUS_AVAILABLE);
    if (lcd_i2c_adapter == NULL) {
        pr_err("I2C Adapter %d not found\n", I2C_BUS_AVAILABLE);
        return -ENODEV;
    }

    lcd_i2c_client = i2c_new_device(lcd_i2c_adapter, &lcd_i2c_board_info);
    if (lcd_i2c_client == NULL) {
        pr_err("Failed to create new I2C device\n");
        i2c_put_adapter(lcd_i2c_adapter);
        return -ENODEV;
    }

    ret = i2c_add_driver(&lcd_driver);
    if (ret < 0) {
        pr_err("Failed to add LCD I2C driver\n");
        i2c_unregister_device(lcd_i2c_client);
        i2c_put_adapter(lcd_i2c_adapter);
        return ret;
    }

    pr_info("LCD Driver Added\n");

    return 0;
}

static void __exit lcd_driver_exit(void) //module exit function
{
    i2c_del_driver(&lcd_driver);
    if (lcd_i2c_client)
        i2c_unregister_device(lcd_i2c_client);
    if (lcd_i2c_adapter)
        i2c_put_adapter(lcd_i2c_adapter);

    pr_info("LCD Driver Removed\n");
}

static void lcd_send_string(const char *str) //test function to send a string to the LCD 
{
    while (*str) {
        lcd_send(*str++, 1); //rs_flag=1 for data
    }
}

module_init(lcd_driver_init);
module_exit(lcd_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ankush<typeankush@gmail.com>");
MODULE_DESCRIPTION("I2C-based HD44780 driver for BBB");

