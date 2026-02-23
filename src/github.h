#ifndef GITHUB_H
#define GITHUB_H

#include <string>
#include <optional>
#include <vector>
#include <utility>
#include <unordered_map>
#include <tuple>

struct RepoInfo {
    std::string name;
    std::string description;
    bool isPrivate;
    std::string htmlUrl;
    std::string sshUrl;
};

class GitHubClient {
public:
    GitHubClient(const std::string& token);
    
    bool authenticate();
    std::string getUsername();
    bool createRepository(const RepoInfo& repo);
    bool repositoryExists(const std::string& name);
    std::vector<RepoInfo> listRepositories();
    bool deleteRepository(const std::string& name);

private:
    std::string token_;
    std::string apiBase_ = "https://api.github.com";
    std::optional<std::string> username_;
    
    std::pair<int, std::string> request(const std::string& method, 
                                         const std::string& path,
                                         const std::string& body = "");
    std::tuple<int, std::string, std::unordered_map<std::string, std::string>> requestWithHeaders(const std::string& method, 
                                         const std::string& path,
                                         const std::string& body = "");
};

#endif
