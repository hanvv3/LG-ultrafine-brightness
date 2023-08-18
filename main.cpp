#include <iostream>
#include <libusb.h>
#include <set>
#include <vector>
#include <unordered_map>
#include <curses.h>

#include "UltrafineDisplay.h"

using std::set;
using std::unordered_map;
using std::vector;
const uint16_t LG_ID_Vendor = 0x43e;

unordered_map<uint16_t, const char *> support_device = {
	{0x9a63, "24MD4KL"},
	{0x9a70, "27MD5KL"},
	{0x9a40, "27MD5KA"}
};

static int get_lg_ultrafine_usb_devices(libusb_device **devs, int usb_cnt, libusb_device ***lg_devs) {
    int lg_cnt = 0;
    struct libusb_device_descriptor desc;

    for (int i = 0; i < usb_cnt; i++) {
        int r = libusb_get_device_descriptor(devs[i], &desc);
	    if(desc.idVendor != LG_ID_Vendor) continue;
	    auto lg_search = support_device.find(desc.idProduct);
	    if (lg_search != support_device.end()) {
	        lg_cnt++;
        }
    }
    if (lg_cnt == 0) return 0;
    *lg_devs = (libusb_device **)malloc(sizeof(libusb_device *) * lg_cnt);

    int k = 0;
    for (int i = 0; i < usb_cnt; i++) {
	    int r = libusb_get_device_descriptor(devs[i], &desc);
	    auto lg_search = support_device.find(desc.idProduct);
	    if (lg_search != support_device.end()) {
	    (*lg_devs)[k++] = devs[i];
	    }
    }

    return lg_cnt;
}

int main(int argc, const char* argv[]) {
    initscr();
    noecho();
    cbreak();

	libusb_device **devs;
    libusb_device **lgdevs;
	libusb_context *ctx = NULL;
	int r;
	int cnt;

	r = libusb_init(&ctx);
	if (r < 0) {
		printw("Failed to initialize 'libusb'. Exiting...\n");
		getch();
		endwin();
		return r;
	}

	cnt = libusb_get_device_list(ctx, &devs);
	if (cnt < 0) {
		printw("Failed to get device list. Try reconnecting the device. Exiting...\n");
		getch();
		endwin();
		return -1;
	}

	int lg_cnt = 0;
	lg_cnt = get_lg_ultrafine_usb_devices(devs, cnt, &lgdevs);

    if (lg_cnt < 1) {
        printw("Error: No device found.\n");
        printw("\t* There must be at least one LG Ultrafine Display.\n");
        printw("\t* Try reconnecting / restarting your device to recognize Thunderbolt I/O.\n");
        getch();
        endwin();
        libusb_free_device_list(devs, 1);
        libusb_exit(NULL);
        return -2;
    }

    if (1 == argc) {
        // If only one display is detected
        if (lg_cnt == 1) {
            UltrafineDisplay display(lgdevs[0]);
            display.interactive();
            display.LG_Close();
        } else {
            int end = false;
            while(!end){
                printw("Found %d LG UltraFine display(s).\n", lg_cnt);
                vector<UltrafineDisplay> Display_pool;
                for (int i = 0; i < lg_cnt; i++) {
                    Display_pool.emplace_back(lgdevs[i]);
                    printw("LG Ultrafine Display %s  brightness %d. [press %d to select]\n", \
                    Display_pool[i].getDisplayName() ,Display_pool[i].get_brightness_level(), i);
                }
                char c = getch();
                if(c == 'q'){
                    printw("Press q to exit\n");
                    break;
                }
                int chooseDisplay = atoi(&c);
                clear();
                Display_pool[chooseDisplay].interactive();
                for(auto &a : Display_pool) {
                    a.LG_Close();
                }
            }
            endwin();
        }

    // New command-line options
    } else if ("--get-brightness" == std::string(argv[1])) {
        if (3 < argc) {
            std::cerr << "Usage: " << argv[0] << " --get-brightness [display_index]" << std::endl;
            libusb_free_device_list(devs, 1);
            return 1;
        } else if (3 == argc) {
            int displayIndex = std::stoi(argv[2]);
            if (displayIndex >= lg_cnt) {
                std::cerr << "Invalid display index." << std::endl;
                libusb_free_device_list(devs, 1);
                return 1;
            }
            UltrafineDisplay display(lgdevs[displayIndex]);
            int brightnessLevel = static_cast<int>(display.get_brightness_level());
            std::cout << displayIndex << "\t" << display.getDisplayName() << "\t\tbrightness: " << brightnessLevel << " ";
        } else {
            for (int i=0; i<lg_cnt; i++) {
                UltrafineDisplay dispArray(lgdevs[i]);
                int brightnessLevel = static_cast<int>(dispArray.get_brightness_level());
                std::cout << i << "\t" << dispArray.getDisplayName() << "\t\tbrightness: " << brightnessLevel << " ";
            }
        }
    } else if ("--set-brightness" == std::string(argv[1])) {
        if (4 == argc) {
            int displayIndex = std::stoi(argv[2]);
            if (displayIndex >= lg_cnt) {
                std::cerr << "Invalid display index." << std::endl;
                libusb_free_device_list(devs, 1);
                return 1;
            }
            uint16_t brightnessValue = std::stoi(argv[3]);
            UltrafineDisplay display(lgdevs[displayIndex]);
            display.set_brightness_level(brightnessValue);
            std::cout << "Brightness set to: " << brightnessValue << std::endl;
        } else {
            std::cerr << "Usage: " << argv[0] << " --set-brightness [display_index] [brightness_value]" << std::endl;
            libusb_free_device_list(devs, 1);
            return 1;
        }
    } else if ("--dim" == std::string(argv[1])) {
        if (2 == argc) {
            UltrafineDisplay display(lgdevs[0]);
            display.dim_step();
        }
    } else if ("--brighten" == std::string(argv[1])) { 
        if (2 == argc) {
            UltrafineDisplay display(lgdevs[0]);
            display.brighten_step();
        }
    } else {
        std::cerr << "Invalid option provided. Use --set-brightness or --get-brightness or provide no options for original functionality." << std::endl;
        libusb_free_device_list(devs, 1);
        return 1;
    }

    // Cleanup
    libusb_free_device_list(devs, 1);
    libusb_exit(NULL);
    return 0;
}
