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
    bool ensureAuth();
    void cmdCreate();
    void cmdAuth();
    void processRepoCreation(const std::string& path);
    
    std::unique_ptr<GitHubClient> client_;
    std::unique_ptr<ConfigManager> config_;
    bool running_;
};

#endif
