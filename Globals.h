#pragma once

#include <unordered_map>

const uint16_t LG_ID_Vendor = 0x43e;

std::unordered_map<uint16_t, const char *> support_device = {
    {0x9a63, "24MD4KL"},
    {0x9a70, "27MD5KL"},
    {0x9a40, "27MD5KA"}
};

// Additional constants and variables can be added here as needed without changing the format.
