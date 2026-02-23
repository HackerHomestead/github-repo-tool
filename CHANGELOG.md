# Changelog

All notable changes to this project will be documented in this file.

## [1.1.0] - 2026-02-23

### Added
- **System Check Command**: New `--check` CLI flag and `check` REPL command to verify:
  - GitHub API authentication status
  - SSH connectivity to GitHub
  - Local git repository status
  - Token permissions and scopes
- **Help Subcommands**: Detailed help for each command via `help <command>`
- **List Filtering**: Case-insensitive filtering in `list` command (e.g., `list foo`)
- **Remote Detection**: Warning when creating a repo in a folder that already has an origin remote
- **getRemoteUrl**: New GitUtils method to get the URL of a remote

### Fixed
- **Pagination**: Improved reliability when fetching large repository lists
- **Null Description**: Fixed crash when repository has null description
- **SSH Push**: Added force push fallback for non-fast-forward pushes

### Changed
- **Improved Error Messages**: More descriptive error messages with troubleshooting suggestions

## [1.0.0] - 2026-02-22

### Added
- Initial release
- Interactive REPL mode
- CLI mode with flags
- Create, list, delete repositories
- SSH push support
- Command history and tab completion
- Token authentication

