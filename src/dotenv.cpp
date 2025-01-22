#include <cstdlib>
#include <fstream>
#include "dotenv.h"

namespace dotenv {

std::string get(const char *name, const char *default_value)
{
    auto dv = default_value == nullptr
        ? std::string()
        : std::string(default_value);
    const char *value = getenv(name);
    if (value != nullptr) {
        return std::string(value);
    } else {
        return dv;
    }
}

std::string get(const std::string &name, const std::string &default_value)
{
    return get(name.c_str(), default_value.c_str());
}

void set(const char *name, const char *value)
{
    setenv(name, value, true);
}

void set(const std::string &name, const std::string &value)
{
    set(name.c_str(), value.c_str());
}

void unset(const char *name)
{
    unsetenv(name);
}

void unset(const std::string &name)
{
    unset(name.c_str());
}

void read(std::istream &stream,
    const std::function<
        void(const std::string &name, const std::string &value)
    > &callback)
{
    std::string line;

    while (std::getline(stream, line)) {
        auto sep_pos = line.find('=');
        if (sep_pos == std::string::npos) {
            continue;
        }

        auto name = line.substr(0, sep_pos);
        if (!name.empty() && name[0] == '#') {
            continue;
        }
        auto value = line.substr(sep_pos + 1);
        callback(name, value);
    }
}

bool load(const char *filename)
{
    std::ifstream stream(filename);
    if (!stream) {
        return false;
    }

    read(stream, [](const std::string &name, const std::string &value) {
        set(name, value);
    });
    return true;
}

bool load(const std::string &filename)
{
    return load(filename.c_str());
}

} 