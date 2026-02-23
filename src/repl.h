#ifndef REPL_H
#define REPL_H

#include <memory>
#include <string>

class GitHubClient;
class ConfigManager;

class REPL {
public:
    REPL();
    void run();
    void runCommand(const std::string& input);
    
private:
    void printBanner();
    void printHelp();
    void printHelpCreate();
    void printHelpList();
    void printHelpDelete();
    void printHelpSsh();
    void printHelpAuth();
    void printHelpCheck();
    bool ensureAuth();
    void cmdCreate();
    void cmdAuth();
    void cmdList(const std::string& filter = "");
    void cmdDelete();
    void cmdSshOnly();
    void cmdCheck(const std::string& path = ".");
    void processRepoCreation(const std::string& path);
    
    std::unique_ptr<GitHubClient> client_;
    std::unique_ptr<ConfigManager> config_;
    bool running_;
};

#endif
