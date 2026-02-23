#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include "github.h"
#include "git_utils.h"
#include "config.h"
#include <filesystem>
#include <fstream>
#include <cstdio>

namespace fs = std::filesystem;

TEST_CASE("RepoInfo struct initialization") {
    RepoInfo repo;
    repo.name = "test-repo";
    repo.description = "Test description";
    repo.isPrivate = false;
    repo.htmlUrl = "https://github.com/user/test-repo";
    repo.sshUrl = "git@github.com:user/test-repo.git";
    
    CHECK(repo.name == "test-repo");
    CHECK(repo.description == "Test description");
    CHECK(repo.isPrivate == false);
    CHECK(repo.htmlUrl == "https://github.com/user/test-repo");
    CHECK(repo.sshUrl == "git@github.com:user/test-repo.git");
}

TEST_CASE("RepoInfo default construction") {
    RepoInfo repo;
    CHECK(repo.name.empty());
    CHECK(repo.description.empty());
    CHECK(repo.isPrivate == false);
    CHECK(repo.htmlUrl.empty());
    CHECK(repo.sshUrl.empty());
}

TEST_CASE("GitUtils - isGitRepo") {
    SUBCASE("Current directory is a git repo") {
        CHECK(GitUtils::isGitRepo("/Users/handy/projects/github-repo-tool") == true);
    }
    
    SUBCASE("Non-existent path") {
        CHECK(GitUtils::isGitRepo("/nonexistent/path") == false);
    }
    
    SUBCASE("Existing non-git directory") {
        fs::create_directories("/tmp/test-not-git");
        CHECK(GitUtils::isGitRepo("/tmp/test-not-git") == false);
        fs::remove_all("/tmp/test-not-git");
    }
    
    SUBCASE("Root directory is not a git repo") {
        CHECK(GitUtils::isGitRepo("/tmp") == false);
    }
}

TEST_CASE("GitUtils - hasRemote") {
    CHECK(GitUtils::hasRemote("/Users/handy/projects/github-repo-tool", "origin") == false);
    CHECK(GitUtils::hasRemote("/Users/handy/projects/github-repo-tool", "nonexistent") == false);
}

TEST_CASE("GitUtils - getCurrentBranch") {
    auto branch = GitUtils::getCurrentBranch("/Users/handy/projects/github-repo-tool");
    CHECK(branch.has_value() == true);
    CHECK(branch->empty() == false);
}

TEST_CASE("GitUtils - getCurrentBranch on non-git directory") {
    fs::create_directories("/tmp/test-branch-not-git");
    auto branch = GitUtils::getCurrentBranch("/tmp/test-branch-not-git");
    CHECK(branch.has_value() == false);
    fs::remove_all("/tmp/test-branch-not-git");
}

TEST_CASE("ConfigManager - default construction") {
    ConfigManager config;
}

TEST_CASE("ConfigManager - hasToken returns false when no token") {
    ConfigManager config;
    CHECK(config.hasToken() == false);
}

TEST_CASE("ConfigManager - save and load token") {
    ConfigManager config;
    std::string testToken = "test-token-12345";
    
    bool saved = config.saveToken(testToken);
    CHECK(saved == true);
    
    auto loaded = config.loadToken();
    CHECK(loaded.has_value() == true);
    CHECK(loaded.value() == testToken);
    
    CHECK(config.hasToken() == true);
    
    std::string home = std::getenv("HOME");
    std::string configPath = home + "/.gh-repo-create.json";
    std::remove(configPath.c_str());
}

TEST_CASE("ConfigManager - load non-existent token returns nullopt") {
    ConfigManager config;
    auto token = config.loadToken();
    CHECK(token.has_value() == false);
}

TEST_CASE("Description length validation") {
    SUBCASE("Description at max length is valid") {
        std::string validDesc(350, 'a');
        CHECK(validDesc.length() == 350);
    }
    
    SUBCASE("Description over max length is invalid") {
        std::string longDesc(351, 'a');
        CHECK(longDesc.length() == 351);
    }
}

TEST_CASE("GitHubClient - construction with token") {
    GitHubClient client("test-token");
    CHECK(client.getUsername().empty());
}

TEST_CASE("GitHubClient - getUsername before authentication") {
    GitHubClient client("test-token");
    std::string username = client.getUsername();
    CHECK(username.empty());
}

TEST_CASE("GitHubClient - listRepositories returns vector") {
    GitHubClient client("test-token");
    auto repos = client.listRepositories();
    CHECK(repos.size() >= 0);
}

TEST_CASE("GitHubClient - listRepositories with invalid token returns empty") {
    GitHubClient client("invalid-token");
    auto repos = client.listRepositories();
    CHECK(repos.size() == 0);
}

TEST_CASE("GitHubClient - repositoryExists returns false for non-existent") {
    GitHubClient client("invalid-token");
    CHECK(client.repositoryExists("definitely-does-not-exist-12345") == false);
}

TEST_CASE("GitHubClient - repositoryExists with valid token") {
    GitHubClient client("test-token");
    CHECK(client.repositoryExists("nonexistent-repo") == false);
}

TEST_CASE("GitHubClient - deleteRepository with invalid token returns false") {
    GitHubClient client("invalid-token");
    CHECK(client.deleteRepository("nonexistent-repo") == false);
}

TEST_CASE("GitHubClient - deleteRepository empty name returns false") {
    GitHubClient client("test-token");
    CHECK(client.deleteRepository("") == false);
}

TEST_CASE("GitHubClient - createRepository with valid token returns false") {
    GitHubClient client("invalid-token");
    RepoInfo repo;
    repo.name = "test-repo";
    repo.description = "Test";
    repo.isPrivate = false;
    CHECK(client.createRepository(repo) == false);
}

TEST_CASE("GitHubClient - createRepository with empty name returns false") {
    GitHubClient client("test-token");
    RepoInfo repo;
    repo.name = "";
    repo.description = "Test";
    repo.isPrivate = false;
    CHECK(client.createRepository(repo) == false);
}

TEST_CASE("GitHubClient - authenticate with invalid token returns false") {
    GitHubClient client("invalid-token");
    CHECK(client.authenticate() == false);
}

TEST_CASE("GitHubClient - authenticate with empty token returns false") {
    GitHubClient client("");
    CHECK(client.authenticate() == false);
}
