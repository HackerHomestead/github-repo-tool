#ifndef GITHUB_H
#define GITHUB_H

#include <string>
#include <optional>
#include <vector>
#include <utility>

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

private:
    std::string token_;
    std::string apiBase_ = "https://api.github.com";
    std::optional<std::string> username_;
    
    std::pair<int, std::string> request(const std::string& method, 
                                         const std::string& path,
                                         const std::string& body = "");
};

#endif
