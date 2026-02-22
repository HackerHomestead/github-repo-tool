#include "repl.h"
#include "github.h"
#include "config.h"
#include "git_utils.h"
#include <iostream>
#include <sstream>
#include <readline/readline.h>
#include <readline/history.h>
#include <algorithm>
#include <cctype>

namespace {
const std::string RESET = "\033[0m";
const std::string BOLD = "\033[1m";
const std::string BLUE = "\033[34m";
const std::string GREEN = "\033[32m";
const std::string RED = "\033[31m";
const std::string YELLOW = "\033[33m";
const std::string GRAY = "\033[90m";

std::string trim(const std::string& s) {
    auto start = s.begin();
    while (start != s.end() && std::isspace(*start)) start++;
    auto end = s.end();
    do { end--; } while (std::distance(start, end) > 0 && std::isspace(*end));
    return std::string(start, end + 1);
}

bool isValidRepoName(const std::string& name) {
    if (name.empty() || name.length() > 100) return false;
    for (char c : name) {
        if (!std::isalnum(c) && c != '-' && c != '_') return false;
    }
    return true;
}
}

REPL::REPL() : running_(false) {
    config_ = std::make_unique<ConfigManager>();
}

void REPL::printBanner() {
    std::cout << BLUE << R"(
  ____ _                 _ _    ____            _     ____            
 / ___| | ___  ___ _   _| | | _/ ___| _   _ ___| |_  / ___| _   _ ___ 
| |   | |/ _ \/ __| | | | | |/ \___ \| | | / __| __| \___ \| | | / __|
| |___| |  __/ (__| |_| | |   |____) | |_| \__ \ |_  ____) | |_| \__ \
 \____|_|\___|\___|\__,_|_|_|  |____/ \__, |___/\__| |____/ \__, |___/
                                       |___/                |___/ 
)" << RESET;
    std::cout << BOLD << "  gh-repo-create v1.0.0" << RESET << GRAY << " - GitHub Repository Creator\n" << RESET;
    std::cout << GRAY << "  Type 'help' for available commands\n\n" << RESET;
}

void REPL::printHelp() {
    std::cout << BOLD << "Available commands:\n" << RESET;
    std::cout << "  " << GREEN << "create" << RESET << " (c)  - Create a new GitHub repository\n";
    std::cout << "  " << GREEN << "auth" << RESET << "       - Manage authentication\n";
    std::cout << "  " << GREEN << "help" << RESET << "      - Show this help message\n";
    std::cout << "  " << GREEN << "exit" << RESET << "      - Exit the REPL\n";
}

bool REPL::ensureAuth() {
    auto token = config_->loadToken();
    if (!token.has_value()) {
        std::cout << YELLOW << "No GitHub token found. Please authenticate first.\n" << RESET;
        cmdAuth();
        token = config_->loadToken();
        if (!token.has_value()) return false;
    }
    
    client_ = std::make_unique<GitHubClient>(token.value());
    if (!client_->authenticate()) {
        std::cout << RED << "Authentication failed. Please check your token and try again.\n" << RESET;
        cmdAuth();
        token = config_->loadToken();
        if (!token.has_value()) return false;
        client_ = std::make_unique<GitHubClient>(token.value());
        if (!client_->authenticate()) return false;
    }
    
    std::cout << GREEN << "Authenticated as: " << client_->getUsername() << RESET << "\n";
    return true;
}

void REPL::cmdAuth() {
    std::cout << "\nTo create a GitHub Personal Access Token:\n";
    std::cout << "  1. Go to https://github.com/settings/tokens\n";
    std::cout << "  2. Click 'Generate new token (classic)'\n";
    std::cout << "  3. Select scopes: 'repo'\n";
    std::cout << "  4. Copy the token and paste below\n\n";
    
    char* input = readline("Enter your GitHub token: ");
    if (!input) return;
    
    std::string token = trim(input);
    free(input);
    
    if (!token.empty()) {
        if (config_->saveToken(token)) {
            std::cout << GREEN << "Token saved successfully!\n" << RESET;
        } else {
            std::cout << RED << "Failed to save token.\n" << RESET;
        }
    }
}

void REPL::processRepoCreation(const std::string& path) {
    if (!GitUtils::isGitRepo(path)) {
        std::cout << RED << "Error: " << path << " is not a git repository\n" << RESET;
        return;
    }
    
    std::cout << "\n" << BOLD + BLUE + "Repository Creation" << RESET << "\n";
    std::cout << std::string(40, '-') << "\n";
    
    std::string repoName;
    while (true) {
        char* input = readline("Repository name: ");
        if (!input) return;
        repoName = trim(input);
        free(input);
        
        if (repoName.empty()) {
            std::cout << RED << "Repository name cannot be empty\n" << RESET;
            continue;
        }
        if (!isValidRepoName(repoName)) {
            std::cout << RED << "Invalid name. Use only letters, numbers, hyphens, and underscores.\n" << RESET;
            continue;
        }
        break;
    }
    
    std::string description;
    while (true) {
        std::cout << "Description (max 350 chars): ";
        std::cout << GRAY << "(press Enter to skip)" << RESET << "\n";
        
        char* input = readline("> ");
        if (!input) return;
        description = trim(input);
        free(input);
        
        if (description.length() > 350) {
            std::cout << RED << "Description too long (" << description.length() << "/350)\n" << RESET;
            continue;
        }
        break;
    }
    
    std::cout << "\nVisibility:\n";
    std::cout << "  1. Public\n";
    std::cout << "  2. Private\n";
    
    bool isPrivate = false;
    while (true) {
        char* input = readline("Choose (1/2): ");
        if (!input) return;
        std::string choice = trim(input);
        free(input);
        
        if (choice == "1") {
            isPrivate = false;
            break;
        } else if (choice == "2") {
            isPrivate = true;
            break;
        }
    }
    
    std::cout << "\n" << BOLD << "Summary:" << RESET << "\n";
    std::cout << "  Name: " << repoName << "\n";
    std::cout << "  Description: " << (description.empty() ? "(none)" : description) << "\n";
    std::cout << "  Visibility: " << (isPrivate ? "Private" : "Public") << "\n\n";
    
    char* confirm = readline("Create repository? (y/n): ");
    if (!confirm) return;
    std::string confirmStr = trim(confirm);
    free(confirm);
    
    if (confirmStr != "y" && confirmStr != "Y") {
        std::cout << "Cancelled.\n";
        return;
    }
    
    std::cout << YELLOW << "Creating repository...\n" << RESET;
    
    RepoInfo repo;
    repo.name = repoName;
    repo.description = description;
    repo.isPrivate = isPrivate;
    
    if (client_->createRepository(repo)) {
        std::cout << GREEN << "Repository created successfully!\n" << RESET;
        
        char* pushConfirm = readline("Push to GitHub? (y/n): ");
        if (!pushConfirm) return;
        std::string pushStr = trim(pushConfirm);
        free(pushConfirm);
        
        if (pushStr == "y" || pushStr == "Y") {
            std::string sshUrl = "git@github.com:" + client_->getUsername() + "/" + repoName + ".git";
            
            if (GitUtils::hasRemote(path, "origin")) {
                GitUtils::setRemoteUrl(path, "origin", sshUrl);
                std::cout << "Updated 'origin' remote\n";
            } else {
                GitUtils::addRemote(path, "origin", sshUrl);
                std::cout << "Added 'origin' remote\n";
            }
            
            auto branch = GitUtils::getCurrentBranch(path);
            if (branch.has_value()) {
                if (GitUtils::push(path, "origin", branch.value())) {
                    std::cout << GREEN << "Pushed successfully!\n" << RESET;
                } else {
                    std::cout << RED << "Push failed.\n" << RESET;
                }
            }
        }
    } else {
        std::cout << RED << "Failed to create repository.\n" << RESET;
    }
}

void REPL::cmdCreate() {
    if (!ensureAuth()) return;
    
    std::cout << "\nEnter the path to your local git repository:\n";
    std::cout << GRAY << "(press Enter to use current directory)\n" << RESET;
    
    char* input = readline("Path: ");
    if (!input) return;
    
    std::string path = trim(input);
    free(input);
    
    if (path.empty()) {
        path = ".";
    }
    
    processRepoCreation(path);
}

void REPL::runCommand(const std::string& input) {
    std::string cmd = trim(input);
    
    if (cmd == "exit" || cmd == "quit") {
        running_ = false;
        std::cout << "Goodbye!\n";
    } else if (cmd == "help" || cmd == "?") {
        printHelp();
    } else if (cmd == "create" || cmd == "c") {
        cmdCreate();
    } else if (cmd == "auth") {
        cmdAuth();
    } else if (!cmd.empty()) {
        std::cout << RED << "Unknown command: " << cmd << "\n" << RESET;
        std::cout << "Type 'help' for available commands\n";
    }
}

void REPL::run() {
    printBanner();
    
    running_ = true;
    while (running_) {
        char* input = readline("gh-repo> ");
        if (!input) break;
        
        std::string line = trim(input);
        if (!line.empty()) {
            add_history(input);
        }
        
        if (!line.empty()) {
            runCommand(line);
        }
        
        free(input);
    }
}
