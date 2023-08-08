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

unordered_map<uint16_t, char *> support_device = {
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

int main(int argc, char* argv[]) {
    initscr();
    noecho();
    cbreak();

	libusb_device **devs;
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
	libusb_device **lgdevs;
	lg_cnt = get_lg_ultrafine_usb_devices(devs, cnt, &lgdevs);

    if (lg_cnt < 1) {
        printw("Unable to get LG screen device.\n");
        printw("Please comfirm there is an UltraFine display here.\n");
        printw("Try reconnecting/restarting your display for your system to recognize Thunderbolt I/O.\n");
        getch();
        endwin();
        libusb_free_device_list(devs, 1);
        libusb_exit(NULL);
        return -2;
    }

    if (argc == 1) {
        // If only one display is detected
        if (lg_cnt == 1) {
            UltrafineDisplay display(lgdevs[0]);
            display.interactive();
            display.LG_Close();
        } else {
            int end = false;
            while(!end){
                printw("Get %d UltraFine display\n", lg_cnt);
                vector<UltrafineDisplay> Display_pool;
                for (int i = 0; i < lg_cnt; i++) {
                    Display_pool.emplace_back(lgdevs[i]);
                    printw("Find Ultrafine Display %s brightness %d. [press %d to choose]\n", Display_pool[i].getDisplayName() ,Display_pool[i].get_brightness_level(), i);
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
    } else {
        // New command-line options
        int displayIndex = std::stoi(argv[2]);
        if (displayIndex >= lg_cnt) {
            std::cerr << "Invalid display index." << std::endl;
            libusb_free_device_list(devs, 1);
            return 1;
        }
        UltrafineDisplay display(lgdevs[displayIndex]);

        if (std::string(argv[1]) == "--get-brightness") {
            if (argc != 3) {
                std::cerr << "Usage: " << argv[0] << " --get-brightness [display_index]" << std::endl;
                libusb_free_device_list(devs, 1);
                return 1;
            }
            std::cout << "Current brightness: " << display.get_brightness_level() << std::endl;
        } else if (std::string(argv[1]) == "--set-brightness") {
            if (argc != 4) {
                std::cerr << "Usage: " << argv[0] << " --set-brightness [display_index] [brightness_value]" << std::endl;
                libusb_free_device_list(devs, 1);
                return 1;
            }
            uint16_t brightnessValue = std::stoi(argv[3]);
            display.set_brightness_level(brightnessValue);
            std::cout << "Brightness set to: " << brightnessValue << std::endl;
        } else {
            std::cerr << "Invalid option provided. Use --set-brightness or --get-brightness or provide no options for original functionality." << std::endl;
            libusb_free_device_list(devs, 1);
            return 1;
        }
    }

    // Cleanup
    libusb_free_device_list(devs, 1);
    libusb_exit(NULL);
    return 0;
}
