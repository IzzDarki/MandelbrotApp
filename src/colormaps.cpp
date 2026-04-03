#include "colormaps.h"

#include <algorithm>
#include <fstream>
#include <vector>
#include <string>

void loadColormaps(const std::string& filepath) {
    if (!colormaps.empty()) return;

    std::ifstream file(filepath, std::ios::binary);
    if (!file) return;

    while (true) {
        char groupBuf[32];
        char nameBuf[32];
        
        // Break if we hit EOF before reading a new entry
        if (!file.read(groupBuf, sizeof(groupBuf))) break;
        file.read(nameBuf, sizeof(nameBuf));

        // Construct std::string (stops at the first null terminator)
        std::string group(groupBuf);
        std::string name(nameBuf);

        // Read the 768 floats
        std::vector<float> colors(256 * 3);
        file.read(reinterpret_cast<char*>(colors.data()), static_cast<std::streamsize>(colors.size() * sizeof(float)));

        // Find the group or create it if it doesn't exist
        auto groupIt = std::find_if(colormaps.begin(), colormaps.end(), 
            [&group](const auto& g) { return g.first == group; });

        if (groupIt == colormaps.end()) {
            colormaps.push_back({group, {}});
            groupIt = std::prev(colormaps.end());
        }

        groupIt->second.push_back({name, std::move(colors)});
    }
}