#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/kernel.h>

// the necessary header for i2c_new_probed_device
#include <linux/i2c-dev.h>

#include "bbb_ioctl.h"  // Include IOCTL header for user-space communication

#define I2C_BUS_AVAILABLE   (   2  )                 // I2C Bus available in BBB
#define SLAVE_DEVICE_NAME   ("PCF8574")              // Device and Driver Name
#define LCD_SLAVE_ADDR      (  0x27 )                // LCD Slave Address (adjust according to your device)

/* I2C Adapter and Client */
static struct i2c_adapter *lcd_i2c_adapter = NULL;   // I2C Adapter Structure
static struct i2c_client  *lcd_i2c_client   = NULL;  // I2C Client Structure

/* Function to send data to the LCD */
static int lcd_send_data(unsigned char data)
{
    unsigned char high_nibble, low_nibble;
    unsigned char buf[4];

    high_nibble = data & 0xF0;         // Higher nibble
    low_nibble = (data << 4) & 0xF0;   // Lower nibble

    // Send the higher nibble
    buf[0] = high_nibble | 0x0D;       // RS = 1 (data mode), RW = 0 (write), E = 1 (enable)
    buf[1] = high_nibble | 0x09;       // RS = 1, RW = 0, E = 0 (disable)
    if (i2c_master_send(lcd_i2c_client, buf, 2) < 0) {
         pr_err("Error sending high nibble, return code: %d\n", i2c_master_send(lcd_i2c_client, buf, 2));
        return -1;
    }

    // Delay (same as in lcd.c)
    usleep_range(100, 150);  // 40-50µs delay as required for the LCD //changed from (40,50)

    // Send the lower nibble
    buf[0] = low_nibble | 0x0D;        // RS = 1 (data mode), RW = 0, E = 1
    buf[1] = low_nibble | 0x09;        // RS = 1, RW = 0, E = 0
    if (i2c_master_send(lcd_i2c_client, buf, 2) < 0) {
        pr_err("Error sending low nibble\n");
        return -1;
    }

    // Delay (same as in lcd.c)
    usleep_range(100, 150);  // 40-50µs delay

    return 0;
}

/* LCD Initialization Function */
static void lcd_init(void)
{
    // Delay before starting the initialization sequence
    msleep(15);   // Wait for more than 15ms after VCC rises to 4.5V

    // Send initialization commands to the LCD
    lcd_send_data(0x30);   // Wake-up command 1
    msleep(5);             // Delay for more than 4.1ms

    lcd_send_data(0x30);   // Wake-up command 2
    usleep_range(100, 150); // Delay for more than 100µs

    lcd_send_data(0x30);   // Wake-up command 3
    usleep_range(100, 150); // Delay for more than 100µs

    lcd_send_data(0x20);   // Set 4-bit mode
    usleep_range(100, 150); // Delay for more than 100µs

    // Function Set: 4-bit mode, 2 lines, 5x8 dots
    lcd_send_data(0x28);   // Function set command
    usleep_range(40, 50);  // Delay for 40-50µs

    // Display ON/OFF Control: display on, cursor off, no blinking
    lcd_send_data(0x0C);   // Display on, cursor off
    usleep_range(40, 50);  // Delay for 40-50µs

    // Clear display
    lcd_send_data(0x01);   // Clear display
    msleep(2);             // Delay for more than 1.53ms (clear operation)

    // Entry Mode Set: increment cursor, no display shift
    lcd_send_data(0x06);   // Entry mode set
    usleep_range(40, 50);  // Delay for 40-50µs
}

/* This function gets called when the slave has been found */
static int lcd_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    pr_info("HD44780 LCD Probed\n");

    // Initialize the LCD
    lcd_init();

    // Clear the LCD (optional)
    lcd_send_data(0x01); // Clear display command
    msleep(2);           // Delay for more than 1.5ms

    return 0;
}

/* This function gets called when the slave is removed */
static int lcd_remove(struct i2c_client *client)
{
    pr_info("HD44780 LCD Removed\n");
    return 0;
}

/* I2C Device ID Structure */
static const struct i2c_device_id lcd_id[] = {
    { SLAVE_DEVICE_NAME, 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c, lcd_id);

/* I2C Driver Structure */
static struct i2c_driver lcd_driver = {
    .driver = {
        .name   = SLAVE_DEVICE_NAME,
        .owner  = THIS_MODULE,
    },
    .probe          = lcd_probe,
    .remove         = lcd_remove,
    .id_table       = lcd_id,
};

/* I2C Board Info Structure */
static struct i2c_board_info lcd_i2c_board_info = {
    I2C_BOARD_INFO(SLAVE_DEVICE_NAME, LCD_SLAVE_ADDR)
};

/* Module Init Function */
static int __init lcd_driver_init(void)
{
    int ret = -1;
    lcd_i2c_adapter = i2c_get_adapter(I2C_BUS_AVAILABLE);
    if (lcd_i2c_adapter != NULL) {
    	// Set the I2C clock speed (bus frequency) to 100 kHz
       // lcd_i2c_adapter->bus_clk_rate = 100000;  // Standard I2C speed
        lcd_i2c_client = i2c_new_device(lcd_i2c_adapter, &lcd_i2c_board_info);
        if (lcd_i2c_client != NULL) {
            i2c_add_driver(&lcd_driver);
            pr_info("LCD Driver Added\n");
            ret = 0;
        } else {
            pr_err("Failed to create new client\n");
        }
        i2c_put_adapter(lcd_i2c_adapter);
    } else {
        pr_err("I2C Bus Adapter Not Available\n");
    }
    return ret;
}

/* Module Exit Function */
static void __exit lcd_driver_exit(void)
{
    if (lcd_i2c_client != NULL) {
        i2c_unregister_device(lcd_i2c_client);
        i2c_del_driver(&lcd_driver);
    }
    pr_info("LCD Driver Removed\n");
}

module_init(lcd_driver_init);
module_exit(lcd_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ankush");
MODULE_DESCRIPTION("I2C-based-HD44780 driver for BBB");

