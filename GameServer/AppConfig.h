#pragma
#include <string>

struct AppSettings {
	std::string	jwtSecret;
	int			tokenExpiryHours;
	std::string	issuer;
	std::string	audience;
	std::string	dbLocalPath;
	std::string	dbXmlPath;
};

AppSettings LoadAppSettings(const std::string& path = "appsettings.json");