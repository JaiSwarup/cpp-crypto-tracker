#ifndef CONFIG_H
#define CONFIG_H

#include <string>

extern std::string CLIENT_ID;
extern std::string CLIENT_SECRET;
extern std::string BASE_URL;

void loadConfig();

#endif // CONFIG_H
