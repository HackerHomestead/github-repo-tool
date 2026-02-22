cmake_minimum_required(VERSION 3.16)
project(gh_repo_create VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

set(READLINE_ROOT "/opt/homebrew/opt/readline")

include(FetchContent)
FetchContent_Declare(
  httplib
  GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
  GIT_TAG v0.15.3
)
FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.10.5
)

FetchContent_MakeAvailable(httplib json)

find_path(READLINE_INCLUDE_DIR readline/readline.h PATHS /opt/homebrew/opt/readline/include)
find_library(READLINE_LIBRARY NAMES readline PATHS /opt/homebrew/opt/readline/lib)
find_library(HISTORY_LIBRARY NAMES history PATHS /opt/homebrew/opt/readline/lib)

add_executable(gh-repo src/main.cpp src/github.cpp src/repl.cpp src/git_utils.cpp src/config.cpp)
target_include_directories(gh-repo PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src 
    ${READLINE_INCLUDE_DIR}
    ${CMAKE_BINARY_DIR}/_deps/json-src/include
    ${CMAKE_BINARY_DIR}/_deps/json-src/single_include
    ${CMAKE_BINARY_DIR}/_deps/httplib-src
)
target_link_libraries(gh-repo PRIVATE ${READLINE_LIBRARY} ${HISTORY_LIBRARY})
