#ifndef DOTENV_H
#define DOTENV_H

#include <functional>
#include <iostream>
#include <string>

namespace dotenv {

std::string get(const char *name, const char *default_value = nullptr);
std::string get(const std::string &name, const std::string &default_value = "");

void set(const char *name, const char *value);
void set(const std::string &name, const std::string &value);
void unset(const char *name);
void unset(const std::string &name);

void read(std::istream &stream,
    const std::function<
        void(const std::string &name, const std::string &value)
    > &callback);
bool load(const char *filename = ".env");
bool load(const std::string &filename = ".env");

} // namespace dotenv

#endif