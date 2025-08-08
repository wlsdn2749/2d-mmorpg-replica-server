#pragma
#include <string>

struct AppSettings {
	std::string jwtSecret;
	int			tokenExpiryHours;
	std::string	issuer;
	std::string	audience;
};

AppSettings LoadAppSettings(const std::string& path = "appsettings.json");