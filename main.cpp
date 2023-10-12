#include <iostream>
#include <libusb.h>
#include <ncurses.h>

#include "UltrafineDisplay.h"
#include "CLIHandler.h"


int main(int argc, char ** argv) {
    // libusb init
    initscr();
    noecho();
    cbreak();

    libusb_device **devs;
    libusb_device **lg_devs;
    libusb_context *ctx = NULL;
    int cnt;
    int r = libusb_init(&ctx);

    if (r < 0) {
        printw("ERROR 101: INITFAILURE.\n\t- Failed to initialize 'libusb'.  Exiting...\n");
        getch();
        endwin();
        return r;
    }
    libusb_set_option(ctx, LIBUSB_OPTION_LOG_LEVEL, 3);
    cnt = libusb_get_device_list(ctx, &devs);
    if (cnt < 0) {
        printw("ERROR 102: GETDEVICELISTFAILURE.\n\t- Failed to get a list of USB devices.  Exiting...\n");
        getch();
        endwin();
        return -1;
    }

    int lg_cnt = 0;
    lg_cnt = UltrafineDisplay::get_lg_ultrafine_usb_devices(devs, cnt, &lg_devs);
    if (lg_cnt < 1) {
        printw("ERROR 303: No Ultrafine displays were found.\n");
        printw("\t* If it's already connected, please try reconnecting or restarting the device to recognize Thunderbolt I/O.\n");
        getch();
        endwin();
        libusb_free_device_list(devs, 1);
        libusb_exit(NULL);
        return -2;
    }

    // CLI argument processing
    ProcessCLIArgs(argc, argv, devs, lg_cnt, lg_devs);

    endwin();
    return 0;
}
