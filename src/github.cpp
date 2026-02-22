#include "github.h"
#include <iostream>
#include <sstream>
#include <nlohmann/json.hpp>
#include <httplib.h>

using json = nlohmann::json;

GitHubClient::GitHubClient(const std::string& token) : token_(token) {}

bool GitHubClient::authenticate() {
    auto [status, body] = request("GET", "/user");
    if (status == 200) {
        try {
            auto data = json::parse(body);
            username_ = data.value("login", "");
            return true;
        } catch (...) {
            return false;
        }
    }
    return false;
}

std::string GitHubClient::getUsername() {
    if (!username_.has_value()) {
        auto [status, body] = request("GET", "/user");
        if (status == 200) {
            try {
                auto data = json::parse(body);
                username_ = data.value("login", "");
            } catch (...) {}
        }
    }
    return username_.value_or("");
}

bool GitHubClient::createRepository(const RepoInfo& repo) {
    json body = {
        {"name", repo.name},
        {"description", repo.description},
        {"private", repo.isPrivate},
        {"auto_init", true}
    };
    
    auto [status, response] = request("POST", "/user/repos", body.dump());
    return status == 201;
}

bool GitHubClient::repositoryExists(const std::string& name) {
    auto [status, body] = request("GET", "/user/repos?per_page=100");
    if (status == 200) {
        try {
            auto repos = json::parse(body);
            for (const auto& r : repos) {
                if (r.value("name", "") == name) {
                    return true;
                }
            }
        } catch (...) {}
    }
    return false;
}

std::pair<int, std::string> GitHubClient::request(const std::string& method, 
                                                    const std::string& path,
                                                    const std::string& body) {
    httplib::Client cli(apiBase_);
    cli.set_default_headers({
        {"Authorization", "Bearer " + token_},
        {"Accept", "application/vnd.github+json"},
        {"X-GitHub-Api-Version", "2022-11-28"}
    });
    
    auto res = cli.Get(path);
    
    if (method == "POST") {
        res = cli.Post(path, body, "application/json");
    } else if (method == "DELETE") {
        res = cli.Delete(path);
    }
    
    if (!res) {
        return {-1, "Network error"};
    }
    
    return {res->status, res->body};
}
