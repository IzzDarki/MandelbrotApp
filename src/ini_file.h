#pragma once
#ifndef MANDELBROT_INIFILE_INCLUDED

#include <string>
#include <fstream>
#include <map>

template <typename Key_t>
class IniFile {

protected:
    std::string path;
    std::map<Key_t, std::string> contents;

public:
    IniFile(const std::string& _path) : path{_path} {
        readContents();
    }

    inline const std::string& get(const Key_t& key) const { return contents.at(key); }

    void set(const Key_t& key, const std::string& value) {
        contents[key] = value;
        writeContents();
    }

    void remove(const Key_t& key) {
        contents.erase(key);
        writeContents();
    }

    inline auto begin() const { return contents.cbegin(); }
    inline auto end() const { return contents.cend(); }

protected:
    void readContents() {
        std::ifstream stream{path};
        std::string line;
        while(std::getline(stream, line)) {
            auto pos = line.find("=");
            int key = std::stoi(line.substr(0, pos));
            std::string value = line.substr(pos + 1);
            contents[key] = value;
        }
        stream.close();
    }

    void writeContents() {
        std::ofstream stream{path, std::ofstream::out | std::ofstream::trunc}; // Open file and delete previous content
        for (const auto& pair : contents) {
            stream << pair.first << "=" << pair.second << std::endl;
        }
        stream.close();
    }
};

#endif
