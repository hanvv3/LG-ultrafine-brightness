#include "CLIHandler.h"
#include "UltrafineDisplay.h"
#include <iostream>
#include <string>
#include <ncurses.h>
#include <vector>


void ProcessCLIArgs(int argc, char** argv, libusb_device **devs, int lg_cnt, libusb_device **lgdevs) {
    // Check if only the program name is provided as argument
    if (argc == 1) {
        if (lg_cnt == 1) {
            UltrafineDisplay display(lgdevs[0]);
            display.interactive();
            display.LG_Close();
        } else {
            bool end = false;
            while(!end) {
                printw("Found %d LG UltraFine display(s).\n", lg_cnt);
                
                std::vector<UltrafineDisplay> Display_pool;
                for (int i = 0; i < lg_cnt; i++) {
                    Display_pool.emplace_back(lgdevs[i]);
                    printw("LG Ultrafine Display %s brightness %d. [press %d to select]\n", Display_pool[i].getDisplayName(), Display_pool[i].get_brightness_level(), i);
                }

                char choice = getch();
                if (choice == 'q') {
                    printw("Press q to exit\n");
                    break;
                }
                
                int chosenDisplay = atoi(&choice);
                clear();
                Display_pool[chosenDisplay].interactive();
                
                for (auto &displayInstance : Display_pool) {
                    displayInstance.LG_Close();
                }
            }
            endwin();
        }
    } else {
        std::string option = argv[1];
        
        if (option == "--get-brightness") {
            if (argc > 3) {
                std::cerr << "Usage: " << argv[0] << " --get-brightness [display_index]" << std::endl;
                libusb_free_device_list(devs, 1);
                exit(1);
            } else if (argc == 3) {
                int displayIndex = std::stoi(argv[2]);
                if (displayIndex >= lg_cnt) {
                    std::cerr << "Invalid display index." << std::endl;
                    libusb_free_device_list(devs, 1);
                    exit(1);
                }
                UltrafineDisplay display(lgdevs[displayIndex]);
                int brightnessLevel = static_cast<int>(display.get_brightness_level());
                std::cout << displayIndex << "\t" << display.getDisplayName() << "\t\tbrightness: " << brightnessLevel << std::endl;
            } else {
                for (int i = 0; i < lg_cnt; i++) {
                    UltrafineDisplay dispArray(lgdevs[i]);
                    int brightnessLevel = static_cast<int>(dispArray.get_brightness_level());
                    std::cout << i << "\t" << dispArray.getDisplayName() << "\t\tbrightness: " << brightnessLevel << std::endl;
                }
            }
        } else if (option == "--set-brightness") {
            if (argc != 4) {
                std::cerr << "Usage: " << argv[0] << " --set-brightness [display_index] [brightness_value]" << std::endl;
                libusb_free_device_list(devs, 1);
                exit(1);
            }
            
            int displayIndex = std::stoi(argv[2]);
            if (displayIndex >= lg_cnt) {
                std::cerr << "Invalid display index." << std::endl;
                libusb_free_device_list(devs, 1);
                exit(1);
            }

            uint16_t brightnessValue = std::stoi(argv[3]);
            UltrafineDisplay display(lgdevs[displayIndex]);
            display.set_brightness_level(brightnessValue);
            std::cout << "Brightness set to: " << brightnessValue << std::endl;
            
        } else if (option == "--dim") {
            UltrafineDisplay display(lgdevs[0]);
            display.dim_step();
        } else if (option == "--brighten") {
            UltrafineDisplay display(lgdevs[0]);
            display.brighten_step();
        } else {
            std::cerr << "Invalid option provided. Use --set-brightness or --get-brightness or provide no options for original functionality." << std::endl;
            libusb_free_device_list(devs, 1);
            exit(1);
        }
    }

    // Cleanup
    libusb_free_device_list(devs, 1);
    libusb_exit(nullptr);
}
