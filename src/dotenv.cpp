#include <cstdlib>
#include <fstream>
#include "dotenv.h"

namespace dotenv {

std::string get(const char *name, const char *default_value)
{
    auto dv = default_value == nullptr
        ? std::string()
        : std::string(default_value);
#ifdef _WIN32
    size_t size = 0;
    getenv_s(&size, nullptr, 0, name);
    if (size > 0) {
        std::string value(size - 1, '\0');
        getenv_s(&size, &value[0], size, name);
        return value;
    } else {
        return dv;
    }
#else
    const char *value = getenv(name);
    if (value != nullptr) {
        return std::string(value);
    } else {
        return dv;
    }
#endif
}

std::string get(const std::string &name, const std::string &default_value)
{
    return get(name.c_str(), default_value.c_str());
}

void set(const char *name, const char *value)
{
#ifdef _WIN32
    auto var = std::string(name) + '=' + value;
    (void)_putenv(var.c_str());
#else
    setenv(name, value, true);
#endif
}

void set(const std::string &name, const std::string &value)
{
    set(name.c_str(), value.c_str());
}

void unset(const char *name)
{
#ifdef _WIN32
    auto var = std::string(name) + '=';
    (void)_putenv(var.c_str());
#else
    unsetenv(name);
#endif
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

} // namespace dotenv