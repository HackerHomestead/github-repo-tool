#ifndef GIT_UTILS_H
#define GIT_UTILS_H

#include <string>
#include <optional>
#include <array>

class GitUtils {
public:
    static bool isGitRepo(const std::string& path);
    static std::optional<std::string> getCurrentBranch(const std::string& path);
    static bool hasRemote(const std::string& path, const std::string& remoteName);
    static bool addRemote(const std::string& path, const std::string& name, const std::string& url);
    static bool push(const std::string& path, const std::string& remote, const std::string& branch);
    static bool setRemoteUrl(const std::string& path, const std::string& remoteName, const std::string& url);
};

#endif
