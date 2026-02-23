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
#include <wordexp.h>
#include <vector>

namespace {
const std::string RESET = "\033[0m";
const std::string BOLD = "\033[1m";
const std::string BLUE = "\033[34m";
const std::string GREEN = "\033[32m";
const std::string RED = "\033[31m";
const std::string YELLOW = "\033[33m";
const std::string GRAY = "\033[90m";

const std::vector<std::string> COMMANDS = {
    "create", "c", "list", "l", "delete", "d", "ssh", "s", "auth", "help", "exit", "quit", "?"
};

std::string getHistoryPath() {
    const char* home = std::getenv("HOME");
    if (home) {
        return std::string(home) + "/.gh-repo-create-history";
    }
    return ".gh-repo-create-history";
}

void loadHistory() {
    std::string path = getHistoryPath();
    read_history(path.c_str());
}

void saveHistory() {
    std::string path = getHistoryPath();
    write_history(path.c_str());
    history_truncate_file(path.c_str(), 100);
}

char* commandGenerator(const char* text, int state) {
    static size_t idx = 0;
    if (state == 0) {
        idx = 0;
    }
    
    while (idx < COMMANDS.size()) {
        std::string cmd = COMMANDS[idx++];
        if (cmd.find(text) == 0) {
            return strdup(cmd.c_str());
        }
    }
    return nullptr;
}

char** commandCompletion(const char* text, int start, int end);
char** pathCompletion(const char* text, int start, int end);

char** commandCompletion(const char* text, int start, int end) {
    if (start == 0) {
        return rl_completion_matches(text, commandGenerator);
    }
    
    if (start > 0) {
        std::string before = std::string(rl_line_buffer, start);
        if (before.find("create") == 0 || before.find("c ") == 0 || 
            before.find("ssh") == 0 || before.find("s ") == 0) {
            return pathCompletion(text, start, end);
        }
    }
    
    return nullptr;
}

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

char** pathCompletion(const char* text, int start, int end);

char** pathCompletion(const char* text, int start, int end) {
    rl_attempted_completion_over = 1;
    
    std::string input(text);
    std::string dir = ".";
    std::string prefix;
    
    size_t lastSlash = input.find_last_of('/');
    if (lastSlash != std::string::npos) {
        dir = input.substr(0, lastSlash);
        if (dir.empty()) dir = "/";
        prefix = input.substr(lastSlash + 1);
    } else {
        prefix = input;
    }
    
    char** matches = nullptr;
    wordexp_t p;
    if (wordexp((dir + "/*").c_str(), &p, 0) == 0) {
        size_t count = 0;
        for (size_t i = 0; i < p.we_wordc; i++) {
            std::string path = p.we_wordv[i];
            std::string name = path.substr(path.find_last_of('/') + 1);
            if (prefix.empty() || name.find(prefix) == 0) {
                count++;
            }
        }
        
        if (count > 0) {
            matches = (char**)malloc((count + 1) * sizeof(char*));
            size_t idx = 0;
            for (size_t i = 0; i < p.we_wordc; i++) {
                std::string path = p.we_wordv[i];
                std::string name = path.substr(path.find_last_of('/') + 1);
                if (prefix.empty() || name.find(prefix) == 0) {
                    bool isDir = false;
                    if (path.back() == '/') {
                        isDir = true;
                        path = path.substr(0, path.length() - 1);
                    }
                    std::string match = path + (isDir ? "/" : "");
                    matches[idx++] = strdup(match.c_str());
                }
            }
            matches[count] = nullptr;
        }
        wordfree(&p);
    }
    
    if (!matches && !prefix.empty()) {
        matches = (char**)malloc(2 * sizeof(char*));
        matches[0] = strdup((prefix + "/").c_str());
        matches[1] = nullptr;
    }
    
    return matches;
}
}

REPL::REPL() : running_(false) {
    config_ = std::make_unique<ConfigManager>();
    using_history();
    loadHistory();
    rl_attempted_completion_function = commandCompletion;
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
    std::cout << "  " << GREEN << "list" << RESET << " (l)   - List your GitHub repositories\n";
    std::cout << "  " << GREEN << "delete" << RESET << " (d)  - Delete a GitHub repository\n";
    std::cout << "  " << GREEN << "ssh" << RESET << "        - Push via SSH only (no API calls)\n";
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
        
        if (client_->repositoryExists(repoName)) {
            std::cout << RED << "Repository '" << repoName << "' already exists on your GitHub account.\n" << RESET;
            std::cout << YELLOW << "Please choose a different name.\n" << RESET;
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

void REPL::cmdList() {
    if (!ensureAuth()) return;
    
    auto repos = client_->listRepositories();
    
    if (repos.empty()) {
        std::cout << YELLOW << "No repositories found.\n" << RESET;
        return;
    }
    
    std::cout << "\n" << BOLD << "Your Repositories:\n" << RESET;
    std::cout << std::string(60, '-') << "\n";
    
    for (const auto& repo : repos) {
        std::string visibility = repo.isPrivate ? RED + "private" + RESET : GREEN + "public" + RESET;
        std::cout << BOLD << repo.name << RESET << " [" << visibility << "]\n";
        if (!repo.description.empty()) {
            std::cout << GRAY << "  " << repo.description << "\n" << RESET;
        }
        std::cout << GRAY << "  " << repo.htmlUrl << "\n" << RESET;
        std::cout << "\n";
    }
    
    std::cout << "Total: " << repos.size() << " repository(ies)\n";
}

void REPL::cmdDelete() {
    if (!ensureAuth()) return;
    
    auto repos = client_->listRepositories();
    
    if (repos.empty()) {
        std::cout << YELLOW << "No repositories to delete.\n" << RESET;
        return;
    }
    
    std::cout << "\n" << BOLD + RED + "Delete Repository" << RESET << "\n";
    std::cout << std::string(40, '-') << "\n";
    
    std::cout << "Select a repository to delete:\n\n";
    for (size_t i = 0; i < repos.size(); i++) {
        std::cout << "  " << (i + 1) << ". " << repos[i].name << "\n";
    }
    std::cout << "\n  0. Cancel\n";
    
    std::string choice;
    while (true) {
        char* input = readline("Choose (number): ");
        if (!input) return;
        choice = trim(input);
        free(input);
        
        if (choice == "0") {
            std::cout << "Cancelled.\n";
            return;
        }
        
        int idx = std::stoi(choice) - 1;
        if (idx < 0 || idx >= static_cast<int>(repos.size())) {
            std::cout << RED << "Invalid selection. Try again.\n" << RESET;
            continue;
        }
        
        std::string repoName = repos[idx].name;
        std::cout << RED << "\nWARNING: This will permanently delete '" << repoName << "'!\n" << RESET;
        std::cout << "This action cannot be undone.\n\n";
        
        char* confirm = readline("Type the repository name to confirm: ");
        if (!confirm) return;
        std::string confirmName = trim(confirm);
        free(confirm);
        
        if (confirmName != repoName) {
            std::cout << RED << "Confirmation failed. Deletion cancelled.\n" << RESET;
            return;
        }
        
        std::cout << YELLOW << "Deleting repository...\n" << RESET;
        
        if (client_->deleteRepository(repoName)) {
            std::cout << GREEN << "Repository '" << repoName << "' deleted successfully!\n" << RESET;
        } else {
            std::cout << RED << "Failed to delete repository.\n" << RESET;
        }
        return;
    }
}

void REPL::cmdSshOnly() {
    std::cout << "\n" << BOLD + BLUE + "SSH Push (No API)" << RESET << "\n";
    std::cout << std::string(40, '-') << "\n";
    
    std::cout << "Enter the path to your local git repository:\n";
    std::cout << GRAY << "(press Enter to use current directory)\n" << RESET;
    
    char* input = readline("Path: ");
    if (!input) return;
    
    std::string path = trim(input);
    free(input);
    
    if (path.empty()) {
        path = ".";
    }
    
    if (!GitUtils::isGitRepo(path)) {
        std::cout << RED << "Error: " << path << " is not a git repository\n" << RESET;
        return;
    }
    
    if (!GitUtils::hasRemote(path, "origin")) {
        std::cout << RED << "Error: No 'origin' remote configured\n" << RESET;
        return;
    }
    
    std::cout << YELLOW << "Configuring SSH for GitHub...\n" << RESET;
    GitUtils::configureSshForGitHub();
    
    auto branch = GitUtils::getCurrentBranch(path);
    if (!branch.has_value()) {
        std::cout << RED << "Error: Could not determine current branch\n" << RESET;
        return;
    }
    
    std::cout << "Pushing to origin/" << branch.value() << "...\n";
    
    if (GitUtils::push(path, "origin", branch.value())) {
        std::cout << GREEN << "Pushed successfully!\n" << RESET;
    } else {
        std::cout << RED << "Push failed.\n" << RESET;
    }
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
    } else if (cmd == "list" || cmd == "l") {
        cmdList();
    } else if (cmd == "delete" || cmd == "d") {
        cmdDelete();
    } else if (cmd == "ssh" || cmd == "s") {
        cmdSshOnly();
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
    
    saveHistory();
}
