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

You will see a welcome banner:

```
  ____ _                 _ _    ____            _     ____            
 / ___| | ___  ___ _   _| | | _/ ___| _   _ ___| |_  / ___| _   _ ___ 
| |   | |/ _ \/ __| | | | | |/ \___ \| | | / __| __| \___ \| | | / __|
| |___| |  __/ (__| |_| | |   |____) | |_| \__ \ |_  ____) | |_| \__ \
 \____|_|\___|\___|\__,_|_|_|  |____/ \__, |___/\__| |____/ \__, |___/
                                       |___/                |___/       
  gh-repo-create v1.0.0 - GitHub Repository Creator
  Type 'help' for available commands

gh-repo> 
```

#### Commands

| Command | Shortcut | Description |
|---------|----------|-------------|
| `create` | `c` | Create a new GitHub repository |
| `list` | `l` | List your GitHub repositories |
| `delete` | `d` | Delete a GitHub repository |
| `ssh` | `s` | Push via SSH only (no API calls) |
| `check` | - | Check API and SSH connectivity |
| `debug` | - | Toggle debug mode (shows sanitized token info) |
| `auth` | - | Manage authentication |
| `help` | `?` | Show help |
| `exit` | `quit` | Exit the REPL |

#### REPL Features

- **Tab Autocomplete**: Press Tab to complete commands and file paths
- **Command History**: Use arrow keys to navigate previous commands
- **Persistent History**: Saved to `~/.gh-repo-create-history`

#### REPL Examples

```bash
gh-repo> help
Available commands:
  create (c)  - Create a new GitHub repository
  list (l)    - List your GitHub repositories
  delete (d)  - Delete a GitHub repository
  ssh         - Push via SSH only (no API calls)
  auth        - Manage authentication
  check       - Check API and SSH connectivity
  debug       - Toggle debug mode
  help        - Show this help message
  exit        - Exit the REPL

gh-repo> list
Your Repositories:
------------------------------------------------------------
my-app [public]
  https://github.com/username/my-app

my-private-app [private]
  My awesome project
  https://github.com/username/my-private-app

Total: 2 repository(ies)

gh-repo> create
Enter repository name: my-new-repo
Enter description (optional): My new project
Visibility (public/private) [public]: public
Creating repository 'my-new-repo'...
Repository created successfully!
Pushed successfully!

gh-repo> exit
```

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

# Check system configuration
./gh-repo --check

# Enable debug output
./gh-repo --debug --check
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
| `--debug` | Enable debug output (shows sanitized token info) |
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
