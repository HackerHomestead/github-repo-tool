#ifndef REPL_H
#define REPL_H

#include <memory>
#include <string>

class GitHubClient;
class ConfigManager;

std::string safeToken(const std::string& token);

class REPL {
public:
    REPL();
    void run();
    void runCommand(const std::string& input);
    void setDebug(bool debug) { debug_ = debug; }
    bool isDebug() const { return debug_; }
    
private:
    void printBanner();
    void printHelp();
    void printHelpCreate();
    void printHelpList();
    void printHelpDelete();
    void printHelpSsh();
    void printHelpAuth();
    void printHelpCheck();
    void printHelpDebug();
    bool ensureAuth();
    void cmdCreate();
    void cmdAuth();
    void cmdList(const std::string& filter = "");
    void cmdDelete();
    void cmdSshOnly();
    void cmdCheck(const std::string& path = ".");
    void processRepoCreation(const std::string& path);
    void cmdDebug(const std::string& args);
    
    std::unique_ptr<GitHubClient> client_;
    std::unique_ptr<ConfigManager> config_;
    bool running_;
    bool debug_;
};

std::string safeToken(const std::string& token);

#endif
