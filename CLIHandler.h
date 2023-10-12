#ifndef CLIHANDLER_H
#define CLIHANDLER_H

#include <libusb.h>

void ProcessCLIArgs(int argc, char** argv, libusb_device **devs, int lg_cnt, libusb_device **lgdevs);

#endif // CLIHANDLER_H
