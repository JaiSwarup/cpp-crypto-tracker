#ifndef CONFIG_H
#define CONFIG_H

#include <string>

extern std::string CLIENT_ID;
extern std::string CLIENT_SECRET;
extern std::string BASE_URL;
extern std::string WEB_SOCKET_URL;

void loadConfig();

#endif