# gh-repo-create

A modern C++ CLI tool for creating and managing GitHub repositories from the command line, featuring an interactive REPL.

## Features

- **Interactive REPL Mode** - Run without arguments to enter an interactive shell
- **CLI Mode** - Use flags for scripting and automation
- **GitHub API Integration** - Creates, lists, and deletes repos via GitHub REST API
- **SSH Push Support** - Automatically adds remote and pushes your code via SSH
- **SSH-Only Mode** - Push to existing repos without GitHub API (no token needed)
- **System Check** - Verify API and SSH access with `--check` or `check` command
- **Token Authentication** - Securely stores your GitHub Personal Access Token
- **Command History** - Persistent history across sessions (saved to `~/.gh-repo-create-history`)
- **Tab Autocomplete** - Auto-complete commands and file paths
- **Smart Create** - Warns if folder already has a remote configured

## Prerequisites (macOS)

```bash
# Install CMake and dependencies
brew install cmake pkg-config openssl

# readline is already installed on macOS
```

## Build Instructions

```bash
# Clone the repository
git clone <repo-url>
cd github-repo-tool

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build
make -j4
```

## Usage

### Interactive REPL Mode

```bash
./gh-repo
```

Commands:
- `create` (or `c`) - Create a new GitHub repository
- `list` (or `l`) - List your GitHub repositories
- `delete` (or `d`) - Delete a GitHub repository
- `ssh` (or `s`) - Push via SSH only (no API calls)
- `check` - Check API and SSH connectivity
- `auth` - Manage authentication
- `help` - Show help
- `exit` - Exit the REPL

### CLI Mode

```bash
# Create a public repository and push
./gh-repo -p ./my-project -n my-repo --public

# Create a private repository with description
./gh-repo -p . -n new-repo -d "My project description" --private

# List all repositories
./gh-repo --list

# Delete a repository
./gh-repo --delete my-repo

# Push via SSH only (no API, uses existing origin remote)
./gh-repo --ssh-only -p .
```

#### Options

| Flag | Description |
|------|-------------|
| `-p, --path <dir>` | Path to local git repository |
| `-n, --name <name>` | Repository name |
| `-d, --description <d>` | Repository description (max 350 chars) |
| `--public` | Make repository public |
| `--private` | Make repository private |
| `-l, --list` | List all your GitHub repositories |
| `-D, --delete <name>` | Delete a repository by name |
| `--ssh-only` | Skip GitHub API, just push via SSH |
| `--check` | Check API and SSH connectivity |
| `-h, --help` | Show help message |

## Authentication

### Environment Variable (Recommended)

```bash
export GH_TOKEN="your_github_personal_access_token"
```

### Interactive Authentication

Run `./gh-repo` and use the `auth` command to save your token locally.

### Creating a GitHub Token

1. Go to https://github.com/settings/tokens
2. Click "Generate new token (classic)"
3. Select the `repo` scope
4. For delete functionality, also select `delete_repo`
5. Copy and use the token

## SSH Setup

The tool uses SSH for git push operations. Make sure you have:

1. An SSH key added to your GitHub account
2. The SSH key added to your SSH agent:
   ```bash
   ssh-add ~/.ssh/id_ed25519
   ```

## Configuration

### Token File Format

The token is stored in `~/.gh-repo-create.json`:

```json
{"token": "your_github_personal_access_token"}
```

Alternatively, use the `GH_TOKEN` environment variable.

### Other Settings

- History: `~/.gh-repo-create-history`
- GitHub username fallback: `git config --global github.user <username>`

## License

MIT
