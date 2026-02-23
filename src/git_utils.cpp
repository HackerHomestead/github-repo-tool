#include "git_utils.h"
#include <iostream>
#include <cstdlib>
#include <memory>
#include <optional>

static std::string runCommand(const std::string& cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) return "";
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    if (!result.empty() && result.back() == '\n') {
        result.pop_back();
    }
    return result;
}

static int runCommandExit(const std::string& cmd) {
    return system(cmd.c_str());
}

void GitUtils::configureSshForGitHub() {
    runCommandExit("git config --global url.\"git@github.com:\".insteadOf \"https://github.com/\"");
}

bool GitUtils::isGitRepo(const std::string& path) {
    std::string cmd = "cd \"" + path + "\" && git rev-parse --git-dir > /dev/null 2>&1";
    return runCommandExit(cmd) == 0;
}

std::optional<std::string> GitUtils::getCurrentBranch(const std::string& path) {
    std::string cmd = "cd \"" + path + "\" && git rev-parse --abbrev-ref HEAD";
    std::string branch = runCommand(cmd);
    return branch.empty() ? std::nullopt : std::make_optional(branch);
}

bool GitUtils::hasRemote(const std::string& path, const std::string& remoteName) {
    std::string cmd = "cd \"" + path + "\" && git remote show " + remoteName + " 2>/dev/null";
    return runCommandExit(cmd) == 0;
}

bool GitUtils::addRemote(const std::string& path, const std::string& name, const std::string& url) {
    std::string cmd = "cd \"" + path + "\" && git remote add " + name + " " + url;
    return runCommandExit(cmd) == 0;
}

bool GitUtils::push(const std::string& path, const std::string& remote, const std::string& branch) {
    std::string cmd = "cd \"" + path + "\" && git push -u " + remote + " " + branch;
    if (runCommandExit(cmd) != 0) {
        cmd = "cd \"" + path + "\" && git push -u " + remote + " " + branch + " --force";
        return runCommandExit(cmd) == 0;
    }
    return true;
}

bool GitUtils::setRemoteUrl(const std::string& path, const std::string& remoteName, const std::string& url) {
    std::string cmd = "cd \"" + path + "\" && git remote set-url " + remoteName + " " + url;
    return runCommandExit(cmd) == 0;
}
