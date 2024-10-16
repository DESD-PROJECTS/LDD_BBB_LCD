#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

int main() {
    int file;
    char *filename = "/dev/i2c-2";
    int addr = 0x27; // LCD address

    // Open I2C device
    if ((file = open(filename, O_RDWR)) < 0) {
        perror("Failed to open the i2c bus");
        return 1;
    }

    // Set I2C slave address
    if (ioctl(file, I2C_SLAVE, addr) < 0) {
        perror("Failed to acquire bus access and/or talk to slave");
        return 1;
    }

    // Example command to send data (replace with your commands)
    char buf[2] = {0x40, 'H'}; // 0x40 for data mode and 'H'
    if (write(file, buf, 2) != 2) {
        perror("Failed to write to the i2c bus");
        return 1;
    }

    close(file);
    return 0;
}

