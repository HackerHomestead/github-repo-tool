#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <optional>
#include <utility>

struct Config {
    std::string token;
    std::string configPath;
};

class ConfigManager {
public:
    ConfigManager();
    
    std::optional<std::string> loadToken();
    bool saveToken(const std::string& token);
    bool hasToken();
    
private:
    std::string getConfigPath();
    std::string expandTilde(const std::string& path);
    
    std::string configPath_;
};

#endif
