#include "app_utility.h"

void copyStringToBuffer(const std::string& string, char* buffer, unsigned int size) {
    auto index = string.copy(buffer, size - 1);
    buffer[index] = '\0';
}

unsigned long hashDjb2(const std::string& str) {
    unsigned long hash = 5381;
    for (auto c : str) {
        hash = (hash << 5) + hash + c; /* hash * 33 + c */
    }
    return hash;
}

bool replaceAll(std::string& string, const std::string& search, const std::string& replace) {
    std::string::size_type pos = 0;
    while ((pos = string.find(search, pos)) != std::string::npos) {
        string.replace(pos, search.length(), replace);
        pos += replace.length();
    }
    
    return (pos != 0);
}

std::string readFileToString(const char* filePath) {
    std::ifstream inputStream(filePath);
    //inputStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    std::stringstream stringStream; 
    stringStream << inputStream.rdbuf();
    return stringStream.str();
}
