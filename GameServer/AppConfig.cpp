#include "pch.h"
#include "AppConfig.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <picojson/picojson.h>

static std::string read_all(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) throw std::runtime_error("cannot open " + path);
    std::ostringstream oss; oss << f.rdbuf();
    return oss.str();
}

AppSettings LoadAppSettings(const std::string& path)
{
    AppSettings cfg;

    auto text = read_all(path);

    picojson::value v;
    std::string err = picojson::parse(v, text);
    if (!err.empty()) throw std::runtime_error("json parse error: " + err);

    if (!v.is<picojson::object>()) throw std::runtime_error("root must be object");
    const auto& root = v.get<picojson::object>();

    auto it = root.find("AppSettings");
    if (it == root.end() || !it->second.is<picojson::object>())
        throw std::runtime_error("AppSettings missing");

    const auto& app = it->second.get<picojson::object>();

    auto get_str = [&](const char* k, const std::string& dflt) {
        auto it = app.find(k);
        if (it != app.end() && it->second.is<std::string>()) return it->second.get<std::string>();
        return dflt;
        };
    auto get_int = [&](const char* k, int dflt) {
        auto it = app.find(k);
        if (it != app.end() && it->second.is<double>()) return static_cast<int>(it->second.get<double>());
        return dflt;
        };

    cfg.jwtSecret = get_str("JwtSecret", cfg.jwtSecret);
    cfg.tokenExpiryHours = get_int("TokenExpiryHours", cfg.tokenExpiryHours);
    cfg.issuer = get_str("Issuer", cfg.issuer);
    cfg.audience = get_str("Audience", cfg.audience);

    if (cfg.jwtSecret.empty())
        throw std::runtime_error("JwtSecret is empty");

    return cfg;
}