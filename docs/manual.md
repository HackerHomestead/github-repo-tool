# gh-repo-create User Manual

## Table of Contents

1. [Introduction](#introduction)
2. [Installation](#installation)
3. [Configuration](#configuration)
4. [Quick Start](#quick-start)
5. [REPL Mode](#repl-mode)
6. [CLI Mode](#cli-mode)
7. [Examples](#examples)
8. [SSH Setup](#ssh-setup)
9. [Troubleshooting](#troubleshooting)

---

## Introduction

gh-repo-create is a C++ CLI tool for creating and managing GitHub repositories from the command line. It features both an interactive REPL mode and CLI flags for scripting.

### Key Features

- Create, list, and delete GitHub repositories
- Automatic SSH push after repository creation
- SSH-only mode for pushing without API token
- Interactive REPL with command history and autocomplete
- Secure token storage

---

## Installation

### Prerequisites

**macOS:**
```bash
brew install cmake pkg-config openssl readline
```

**Ubuntu/Debian:**
```bash
sudo apt-get install cmake pkg-config libssl-dev libreadline-dev
```

**Build from source:**
```bash
git clone https://github.com/HackerHomestead/github-repo-tool.git
cd github-repo-tool
mkdir build && cd build
cmake ..
make -j4
```

### Install (optional)

```bash
sudo make install
```

Or use the provided install script:
```bash
./install.sh
```

---

## Configuration

### GitHub Token

You need a GitHub Personal Access Token (PAT).

**Option 1: Environment Variable**
```bash
export GH_TOKEN="ghp_xxxxxxxxxxxxxxxxxxxx"
```

**Option 2: Config File**

Create `~/.gh-repo-create.json`:
```json
{"token": "ghp_xxxxxxxxxxxxxxxxxxxx"}
```

### Creating a Token

1. Go to https://github.com/settings/tokens
2. Click "Generate new token (classic)"
3. Select scopes:
   - `repo` - Required for create/list/delete
   - `delete_repo` - Required for delete functionality
4. Copy the token

### SSH Setup

For push operations, configure SSH:

```bash
# Generate SSH key (if needed)
ssh-keygen -t ed25519 -C "your@email.com"

# Add to GitHub: Settings > SSH and GPG keys > New SSH key

# Add to SSH agent
ssh-add ~/.ssh/id_ed25519
```

---

## Quick Start

### First Time Setup

```bash
# Set up your token
export GH_TOKEN="your_token_here"

# Verify authentication
./gh-repo --list
```

### Create and Push a Repository

```bash
# From an existing git project
cd my-project
../gh-repo --name my-project --public
```

### Interactive Mode

```bash
./gh-repo
# Then type commands at the prompt
gh-repo> help
```

---

## REPL Mode

Run without arguments to enter interactive mode:

```bash
./gh-repo
```

### Available Commands

| Command | Shortcut | Description |
|---------|----------|-------------|
| `create` | `c` | Create a new GitHub repository |
| `list` | `l` | List all your repositories |
| `delete` | `d` | Delete a repository |
| `ssh` | `s` | Push via SSH only |
| `auth` | - | Manage authentication |
| `help` | `?` | Show help |
| `exit` | `quit` | Exit the REPL |

### REPL Features

- **Tab Autocomplete**: Press Tab to complete commands and file paths
- **Command History**: Arrow keys navigate previous commands
- **History Persistence**: Saved to `~/.gh-repo-create-history`

---

## CLI Mode

### Options

| Flag | Description |
|------|-------------|
| `-p, --path <dir>` | Path to local git repository (default: current directory) |
| `-n, --name <name>` | Repository name |
| `-d, --description <desc>` | Repository description (max 350 chars) |
| `--public` | Make repository public |
| `--private` | Make repository private |
| `-l, --list` | List all repositories |
| `-D, --delete <name>` | Delete a repository |
| `--ssh-only` | Push via SSH only (skip API) |
| `-h, --help` | Show help |

---

## Examples

### Example 1: Create a New Repository

```bash
# From your project directory
cd ~/projects/my-app

# Create public repo and push
gh-repo --name my-app --public
```

Output:
```
Creating repository 'my-app'...
Repository created successfully!
Pushed successfully!
```

### Example 2: Create Private Repository with Description

```bash
gh-repo -p . -n my-private-app -d "My awesome project" --private
```

### Example 3: List All Repositories

```bash
gh-repo --list
```

Output:
```
Your Repositories:
------------------------------------------------------------
my-app [public]
  https://github.com/username/my-app

my-private-app [private]
  My awesome project
  https://github.com/username/my-private-app

Total: 2 repository(ies)
```

### Example 4: Delete a Repository

```bash
gh-repo --delete old-project
```

### Example 5: SSH-Only Push

When you already have a repository and want to push without API:

```bash
# Configure SSH
gh-repo --ssh-only -p .
```

Useful when:
- You don't have a GitHub token
- You just want to push changes
- Repository already exists on GitHub

### Example 6: Interactive REPL

```bash
$ gh-repo
  ____ _                 _ _    ____            _     ____            
 / ___| | ___  ___ _   _| | | _/ ___| _   _ ___| |_  / ___| _   _ ___ 
| |   | |/ _ \/ __| | | | | |/ \___ \| | | / __| __| \___ \| | | / __|
| |___| |  __/ (__| |_| | |   |____) | |_| \__ \ |_  ____) | |_| \__ \
 \____|_|\___|\___|\__,_|_|_|  |____/ \__, |___/\__| |____/ \__, |___/
                                       |___/                |___/       
  gh-repo-create v1.0.0 - GitHub Repository Creator
  Type 'help' for available commands

gh-repo> help
Available commands:
  create (c)  - Create a new GitHub repository
  list (l)   - List your GitHub repositories
  delete (d)  - Delete a GitHub repository
  ssh         - Push via SSH only (no API calls)
  auth        - Manage authentication
  help        - Show this help message
  exit        - Exit the REPL

gh-repo> list
...
gh-repo> exit
```

---

## SSH Setup

The tool uses SSH for git push operations. Without proper SSH configuration, pushes will fail.

### Check SSH Status

```bash
ssh -T git@github.com
```

### Configure SSH for GitHub

```bash
# Add to ~/.ssh/config
Host github.com
    HostName github.com
    User git
    IdentityFile ~/.ssh/id_ed25519
    AddKeysToAgent yes
```

---

## Troubleshooting

### "No GitHub token found"

Set your token:
```bash
export GH_TOKEN="your_token"
# or
echo '{"token": "your_token"}' > ~/.gh-repo-create.json
```

### "Push failed" / "Permission denied"

1. Check SSH key is added to GitHub
2. Start SSH agent: `ssh-add ~/.ssh/id_ed25519`
3. Verify: `ssh -T git@github.com`

### "Must have admin rights to Repository"

Your token needs `delete_repo` scope. Generate a new token with that permission.

### "Failed to delete repository"

1. Token missing `delete_repo` scope
2. Repository doesn't exist
3. Network issues

### "HTTP 401" / "Bad credentials"

Token is invalid or expired. Generate a new one at https://github.com/settings/tokens

---

## License

MIT
