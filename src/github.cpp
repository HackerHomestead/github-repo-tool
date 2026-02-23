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
    if (username_.has_value() && !username_->empty()) {
        return *username_;
    }
    
    auto [status, body] = request("GET", "/user");
    if (status == 200) {
        try {
            auto data = json::parse(body);
            username_ = data.value("login", "");
            if (username_->empty()) {
                std::cerr << "Warning: API returned empty username\n";
            }
            return *username_;
        } catch (const std::exception& e) {
            std::cerr << "Error parsing username response: " << e.what() << "\n";
        }
    } else {
        std::cerr << "Error fetching username from API: HTTP " << status << "\n";
    }
    
    FILE* pipe = popen("git config --global github.user 2>/dev/null", "r");
    if (pipe) {
        char buffer[256] = {0};
        if (fgets(buffer, sizeof(buffer), pipe)) {
            std::string gitUser = buffer;
            gitUser.erase(gitUser.find_last_not_of(" \n\r") + 1);
            if (!gitUser.empty()) {
                username_ = gitUser;
                std::cerr << "Using username from git config: " << gitUser << "\n";
                pclose(pipe);
                return *username_;
            }
        }
        pclose(pipe);
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

std::vector<RepoInfo> GitHubClient::listRepositories() {
    std::vector<RepoInfo> repos;
    std::string nextPage = "/user/repos?per_page=100";
    
    while (!nextPage.empty()) {
        auto [status, body, headers] = requestWithHeaders("GET", nextPage);
        if (status != 200) break;
        
        try {
            auto data = json::parse(body);
            if (!data.is_array()) {
                std::cerr << "listRepositories: response is not an array\n";
                break;
            }
            for (const auto& r : data) {
                RepoInfo info;
                info.name = r.value("name", "");
                if (r.contains("description") && !r["description"].is_null()) {
                    info.description = r["description"].get<std::string>();
                }
                info.isPrivate = r.value("private", false);
                info.htmlUrl = r.value("html_url", "");
                repos.push_back(info);
            }
        } catch (const std::exception& e) {
            std::cerr << "listRepositories parse error: " << e.what() << "\n";
            break;
        }
        
        nextPage.clear();
        auto linkIt = headers.find("Link");
        if (linkIt != headers.end()) {
            std::string linkHeader = linkIt->second;
            std::cerr << "Link header: " << linkHeader.substr(0, 200) << "\n";
            size_t nextPos = linkHeader.find("rel=\"next\"");
            if (nextPos != std::string::npos) {
                size_t start = linkHeader.rfind("<", nextPos);
                size_t end = linkHeader.find(">", nextPos);
                if (start != std::string::npos && end != std::string::npos) {
                    std::string url = linkHeader.substr(start + 1, end - start - 1);
                    size_t queryPos = url.find("/user/repos");
                    if (queryPos != std::string::npos) {
                        nextPage = url.substr(queryPos);
                        std::cerr << "Next page: " << nextPage << "\n";
                    }
                }
            }
        }
    }
    return repos;
}

bool GitHubClient::deleteRepository(const std::string& name) {
    std::string owner = getUsername();
    if (owner.empty()) {
        std::cerr << "Error: Unable to determine GitHub username\n";
        return false;
    }
    auto [status, body] = request("DELETE", "/repos/" + owner + "/" + name);
    return status == 204;
}

std::pair<int, std::string> GitHubClient::request(const std::string& method, 
                                                    const std::string& path,
                                                    const std::string& body) {
    auto [status, response, headers] = requestWithHeaders(method, path, body);
    return {status, response};
}

std::tuple<int, std::string, std::unordered_map<std::string, std::string>> GitHubClient::requestWithHeaders(const std::string& method, 
                                                    const std::string& path,
                                                    const std::string& body) {
    httplib::Client cli(apiBase_);
    cli.set_default_headers({
        {"Authorization", "Bearer " + token_},
        {"Accept", "application/vnd.github+json"},
        {"X-GitHub-Api-Version", "2022-11-28"}
    });
    
    auto res = method == "GET" ? cli.Get(path) :
               method == "POST" ? cli.Post(path, body, "application/json") :
               cli.Delete(path);
    
    if (!res) {
        return {-1, "Network error", {}};
    }
    
    std::unordered_map<std::string, std::string> headers;
    for (const auto& h : res->headers) {
        headers[h.first] = h.second;
    }
    
    return {res->status, res->body, headers};
}
