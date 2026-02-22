#include "repl.h"
#include "github.h"
#include "config.h"
#include "git_utils.h"
#include <iostream>
#include <string>
#include <cstring>

void printUsage(const char* progName) {
    std::cout << R"(gh-repo-create - Create GitHub repositories from the command line

Usage: )" << progName << R"( [options]

Options:
  -p, --path <dir>       Path to local git repository
  -n, --name <name>      Repository name
  -d, --description <d> Repository description
      --public           Make repository public
      --private          Make repository private
  -P, --push             Push to remote after creation
  -h, --help             Show this help message

Examples:
  )" << progName << R"( --path ./my-project --name my-repo --public
  )" << progName << R"( -p . -n new-repo -d "My project" --private --push

Or run without arguments to enter interactive REPL mode.
)";
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        REPL repl;
        repl.run();
        return 0;
    }
    
    std::string path, name, description;
    bool isPrivate = false;
    bool shouldPush = false;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printUsage(argv[0]);
            return 0;
        } else if ((strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--path") == 0) && i + 1 < argc) {
            path = argv[++i];
        } else if ((strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--name") == 0) && i + 1 < argc) {
            name = argv[++i];
        } else if ((strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--description") == 0) && i + 1 < argc) {
            description = argv[++i];
        } else if (strcmp(argv[i], "--public") == 0) {
            isPrivate = false;
        } else if (strcmp(argv[i], "--private") == 0) {
            isPrivate = true;
        } else if (strcmp(argv[i], "-P") == 0 || strcmp(argv[i], "--push") == 0) {
            shouldPush = true;
        }
    }
    
    if (path.empty()) path = ".";
    if (name.empty()) {
        std::cerr << "Error: Repository name is required\n";
        printUsage(argv[0]);
        return 1;
    }
    
    if (description.length() > 350) {
        std::cerr << "Error: Description too long (max 350 characters)\n";
        return 1;
    }
    
    ConfigManager config;
    auto token = config.loadToken();
    if (!token.has_value()) {
        std::cerr << "Error: No GitHub token found. Set GH_TOKEN env var or run without args to authenticate.\n";
        return 1;
    }
    
    if (!GitUtils::isGitRepo(path)) {
        std::cerr << "Error: " << path << " is not a git repository\n";
        return 1;
    }
    
    GitHubClient client(token.value());
    if (!client.authenticate()) {
        std::cerr << "Error: Authentication failed\n";
        return 1;
    }
    
    if (client.repositoryExists(name)) {
        std::cerr << "Error: Repository '" << name << "' already exists on your GitHub account.\n";
        return 1;
    }
    
    RepoInfo repo;
    repo.name = name;
    repo.description = description;
    repo.isPrivate = isPrivate;
    
    std::cout << "Creating repository '" << name << "'...\n";
    
    if (client.createRepository(repo)) {
        std::cout << "Repository created successfully!\n";
        
        if (shouldPush) {
            std::string sshUrl = "git@github.com:" + client.getUsername() + "/" + name + ".git";
            
            if (GitUtils::hasRemote(path, "origin")) {
                GitUtils::setRemoteUrl(path, "origin", sshUrl);
            } else {
                GitUtils::addRemote(path, "origin", sshUrl);
            }
            
            auto branch = GitUtils::getCurrentBranch(path);
            if (branch.has_value()) {
                if (GitUtils::push(path, "origin", branch.value())) {
                    std::cout << "Pushed successfully!\n";
                } else {
                    std::cerr << "Push failed\n";
                    return 1;
                }
            }
        }
    } else {
        std::cerr << "Failed to create repository\n";
        return 1;
    }
    
    return 0;
}
