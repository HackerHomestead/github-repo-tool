#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <string>

namespace constants {

// GitHub API
constexpr int API_REPOS_PER_PAGE = 100;
constexpr int MAX_DESCRIPTION_LENGTH = 350;
constexpr int MAX_REPO_NAME_LENGTH = 100;

// Git
constexpr const char* DEFAULT_REMOTE = "origin";
constexpr const char* DEFAULT_BRANCH = "main";

// Config
constexpr const char* CONFIG_FILE = ".gh-repo-create.json";
constexpr const char* HISTORY_FILE = ".gh-repo-create-history";
constexpr const char* GIT_CONFIG_USER_KEY = "github.user";

// Timeouts (seconds)
constexpr int SSH_TIMEOUT = 30;
constexpr int API_TIMEOUT = 60;

}  // namespace constants

#endif  // CONSTANTS_H
