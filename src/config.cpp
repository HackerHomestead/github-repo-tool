#include "config.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstdlib>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

ConfigManager::ConfigManager() {
    configPath_ = getConfigPath();
}

std::string ConfigManager::getConfigPath() {
    std::string home = expandTilde("~/.gh-repo-create.json");
    return home;
}

std::string ConfigManager::expandTilde(const std::string& path) {
    if (path.substr(0, 2) == "~" + std::string(1, std::filesystem::path::preferred_separator)) {
        const char* home = std::getenv("HOME");
        if (home) {
            return std::string(home) + path.substr(1);
        }
    }
    return path;
}

std::optional<std::string> ConfigManager::loadToken() {
    const char* envToken = std::getenv("GH_TOKEN");
    if (envToken) {
        return std::string(envToken);
    }
    
    std::ifstream file(configPath_);
    if (!file.is_open()) {
        return std::nullopt;
    }
    
    try {
        json data;
        file >> data;
        if (data.contains("token")) {
            return data["token"].get<std::string>();
        }
    } catch (...) {}
    
    return std::nullopt;
}

bool ConfigManager::saveToken(const std::string& token) {
    json data;
    data["token"] = token;
    
    std::ofstream file(configPath_);
    if (!file.is_open()) {
        return false;
    }
    
    file << data.dump(2);
    return true;
}

bool ConfigManager::hasToken() {
    return loadToken().has_value();
}
