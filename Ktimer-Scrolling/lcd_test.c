#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include "bbb_lcd.h"
#include "bbb_ioctl.h"

int main(int argc, char *argv[])  // Change 'void *argv[]' to 'char *argv[]'
{
    int choice, len, fd, ret;
    char buf[32];
    struct ioctl_msg msg;

    fd = open("/dev/bbb_lcd0", O_WRONLY);
    if (fd < 0)
    {
        perror("open() failed\n");
        return fd; // Exit if open fails
    }

    memset(msg.buf, '\0', BUF_SIZE);

    if (argc < 2) {
        printf("Usage: sudo ./a.out <command> [args]\n");
        return 1; // Exit if no command provided
    }

    choice = atoi(argv[1]);

    switch (choice)
    {
    case LCD_CLEAR:
        ret = ioctl(fd, LCD_CLEAR_IOCTL, &msg);
        if (ret != 0)
        {
            perror("LCD clear failed\n");
            return ret;
        }
        printf("ioctl: LCD clear executed\n");
        break;

    case LCD_WRITE:
        if (argc < 3) {
            printf("Usage: sudo ./a.out 1 <data_for_lcd>\n");
            return 1; // Exit if not enough arguments
        }
        strcpy(buf, argv[2]);
        len = strlen(argv[2]);
        ret = write(fd, buf, len);
        if (ret < 0)
        {
            perror("write() failed\n");
        }
        printf("No. of bytes sent: %d, string: %s, len: %d\n", ret, buf, len);
        break;

    case SHIFT_LEFT:
        msg.shift = atoi(argv[2]);
        ret = ioctl(fd, LCD_SHIFT_LEFT, &msg);
        if (ret != 0)
        {
            perror("LCD shift left failed\n");
            return ret;
        }
        printf("ioctl: LCD shift left executed\n");
        break;

    case SHIFT_RIGHT:
        msg.shift = atoi(argv[2]);
        ret = ioctl(fd, LCD_SHIFT_RIGHT, &msg);
        if (ret != 0)
        {
            perror("LCD shift right failed\n");
            return ret;
        }
        printf("ioctl: LCD shift right executed\n");
        break;

    case PRINT_ON_FIRST_LINE:
        if (argc < 3) {
            printf("Usage: sudo ./a.out 4 <data_for_print_on_first_line>\n");
            return 1; // Exit if not enough arguments
        }
        strcpy(msg.buf, argv[2]);
        msg.line_number = 1;
        ret = ioctl(fd, LCD_PRINT_ON_FIRST_LINE, &msg);
        if (ret != 0)
        {
            perror("Print on LCD first line failed\n");
            return ret;
        }
        printf("ioctl: Print on first line of LCD executed\n");
        break;

    case PRINT_ON_SECOND_LINE:
        if (argc < 3) {
            printf("Usage: sudo ./a.out 5 <data_for_print_on_second_line>\n");
            return 1; // Exit if not enough arguments
        }
        strcpy(msg.buf, argv[2]);
        msg.line_number = 2;
        ret = ioctl(fd, LCD_PRINT_ON_SECOND_LINE, &msg);
        if (ret != 0)
        {
            perror("Print on LCD second line failed\n");
            return ret;
        }
        printf("ioctl: Print on second line of LCD executed\n");
        break;

    // New case for scrolling left
    case SCROLL_LEFT:
        if (argc < 3) {
            printf("Usage: sudo ./a.out 6 <number_of_scrolls>\n");
            return 1; // Exit if not enough arguments
        }
        msg.shift = atoi(argv[2]);
        ret = ioctl(fd, LCD_START_SCROLL_LEFT, &msg); // Assuming LCD_SCROLL_LEFT is defined
        if (ret != 0)
        {
            perror("LCD scroll left failed\n");
            return ret;
        }
        printf("ioctl: LCD scroll left executed\n");
        break;

    // New case for scrolling right
    case SCROLL_RIGHT:
        if (argc < 3) {
            printf("Usage: sudo ./a.out 7 <number_of_scrolls>\n");
            return 1; // Exit if not enough arguments
        }
        msg.shift = atoi(argv[2]);
        ret = ioctl(fd, LCD_START_SCROLL_RIGHT, &msg); // Assuming LCD_SCROLL_RIGHT is defined
        if (ret != 0)
        {
            perror("LCD scroll right failed\n");
            return ret;
        }
        printf("ioctl: LCD scroll right executed\n");
        break;
        case STOP_SCROLL:
        if (argc < 3) {
            printf("Usage: sudo ./a.out 7 <number_of_scrolls>\n");
            return 1; // Exit if not enough arguments
        }
        msg.shift = atoi(argv[2]);
        ret = ioctl(fd, LCD_STOP_SCROLL, &msg); // stop scrolling is defined
        if (ret != 0)
        {
            perror("LCD stop scrolling failed\n");
            return ret;
        }
        printf("ioctl: LCD scroll stoped\n");
        break;

    default:
        printf("Invalid command given. Below is the right way to provide commands for LCD:\n");
        printf("sudo ./a.out 0 <====== lcd clear\n");
        printf("sudo ./a.out 1 data_for_lcd <====== lcd_write\n");
        printf("sudo ./a.out 2 number_of_left_shift <====== lcd_left_shift\n");
        printf("sudo ./a.out 3 number_of_right_shift <====== lcd_right_shift\n");
        printf("sudo ./a.out 4 data_for_print_on_first_line <====== print_on_first_line\n");
        printf("sudo ./a.out 5 data_for_print_on_second_line <====== print_on_second_line\n");
        printf("sudo ./a.out 6 number_of_scrolls <====== scroll_left\n");  // Scroll left
        printf("sudo ./a.out 7 number_of_scrolls <====== scroll_right\n"); // Scroll right
        break;
    }

    close(fd);
    return 0;
}

