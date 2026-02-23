cmake_minimum_required(VERSION 3.16)
project(gh_repo_create VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

set(READLINE_ROOT "/opt/homebrew/opt/readline")

set(HTTPLIB_USE_OPENSSL_IF_AVAILABLE ON)
find_package(OpenSSL 3.0 REQUIRED COMPONENTS SSL Crypto)

include(FetchContent)
FetchContent_Declare(
  httplib
  GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
  GIT_TAG v0.18.1
)
FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.10.5
)
FetchContent_Declare(
  doctest
  GIT_REPOSITORY https://github.com/doctest/doctest.git
  GIT_TAG v2.4.11
)

FetchContent_MakeAvailable(httplib json doctest)

find_path(READLINE_INCLUDE_DIR readline/readline.h PATHS /opt/homebrew/opt/readline/include)
find_library(READLINE_LIBRARY NAMES readline PATHS /opt/homebrew/opt/readline/lib)
find_library(HISTORY_LIBRARY NAMES history PATHS /opt/homebrew/opt/readline/lib)

find_library(OPENSSL_LIBRARY NAMES ssl PATHS /opt/homebrew/opt/openssl/lib)
find_library(CRYPTO_LIBRARY NAMES crypto PATHS /opt/homebrew/opt/openssl/lib)

add_executable(gh-repo src/main.cpp src/github.cpp src/repl.cpp src/git_utils.cpp src/config.cpp)
target_include_directories(gh-repo PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src 
    ${READLINE_INCLUDE_DIR}
    ${CMAKE_BINARY_DIR}/_deps/json-src/include
    ${CMAKE_BINARY_DIR}/_deps/json-src/single_include
    ${CMAKE_BINARY_DIR}/_deps/httplib-src
    /opt/homebrew/opt/openssl/include
)
target_compile_definitions(gh-repo PRIVATE CPPHTTPLIB_OPENSSL_SUPPORT)
target_link_libraries(gh-repo PRIVATE ${READLINE_LIBRARY} ${HISTORY_LIBRARY} OpenSSL::SSL OpenSSL::Crypto)

add_executable(test_gh_repo test/main.cpp src/github.cpp src/git_utils.cpp src/config.cpp)
target_link_libraries(test_gh_repo PRIVATE doctest::doctest OpenSSL::SSL OpenSSL::Crypto)
target_compile_definitions(test_gh_repo PRIVATE CPPHTTPLIB_OPENSSL_SUPPORT DOCTEST_CONFIG_LINK)
target_include_directories(test_gh_repo PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src 
    ${CMAKE_BINARY_DIR}/_deps/json-src/include
    ${CMAKE_BINARY_DIR}/_deps/json-src/single_include
    ${CMAKE_BINARY_DIR}/_deps/httplib-src
    ${CMAKE_BINARY_DIR}/_deps/doctest-src
    /opt/homebrew/opt/openssl/include
)
