#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <cstdlib>
#include <cstring>

struct InputDevice
{
    int fd;
    std::string name;
};

std::string execute_shell_command(const char* command)
{
    char buffer[128];
    std::string result = "";

    FILE* pipe = popen(command, "r");
    if (!pipe)
    {
        std::cerr << "Failed to open pipe." << std::endl;
        return "";
    }

    while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
    {
        result += buffer;
    }

    pclose(pipe);
    return result;
}

int main()
{
    // Run the shell command to get the event number for Magic Keyboard
    std::string eventNumber = execute_shell_command("cat /proc/bus/input/devices | grep -B 4 -A 7 'Magic Keyboard' | grep -oP 'event\\d+'");
    eventNumber = eventNumber.substr(eventNumber.find("event"));

    if (eventNumber.empty())
    {
        std::cerr << "ERROR: Apple Magic Keyboard not found." << std::endl;
        return 1;
    }

    // Construct the path to the event device
    const char* devInputPath = "/dev/input";
    std::string eventPath = std::string(devInputPath) + "/" + eventNumber;

    // Open the event device
    int fd = open(eventPath.c_str(), O_RDONLY);
    if (fd == -1)
    {
        perror("ERROR: Failed to open event device");
        std::cerr << eventNumber << "\t" << eventPath;
        return 1;
    }

    input_event event;
    while (true)
    {
        if (read(fd, &event, sizeof(event)) == sizeof(event))
        {
            if (event.type == EV_KEY)
            {
                if (event.code == KEY_BRIGHTNESSDOWN)
                {
                    // Execute the ufctl - command using system function
                    system("ufctl -");
                }
                else if (event.code == KEY_BRIGHTNESSUP)
                {
                    // Execute the ufctl + command using system function
                    system("ufctl +");
                }
            }
        }
    }

    close(fd);

    return 0;
}
