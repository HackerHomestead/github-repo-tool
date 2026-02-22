# gh-repo-create

A modern C++ CLI tool for creating GitHub repositories from the command line, featuring an interactive REPL.

## Features

- **Interactive REPL Mode** - Run without arguments to enter an interactive shell
- **CLI Mode** - Use flags for scripting and automation
- **GitHub API Integration** - Creates repos via GitHub REST API
- **Git Push Support** - Automatically adds remote and pushes your code
- **Token Authentication** - Securely stores your GitHub Personal Access Token

## Prerequisites (macOS)

```bash
# Install CMake and pkg-config
brew install cmake pkg-config

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
cmake -DCMAKE_POLICY_VERSION_MINIMUM=3.5 ..

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
- `auth` - Manage authentication
- `help` - Show help
- `exit` - Exit the REPL

### CLI Mode

```bash
# Create a public repository and push
./gh-repo -p ./my-project -n my-repo --public --push

# Create a private repository with description
./gh-repo -p . -n new-repo -d "My project description" --private --push
```

#### Options

| Flag | Description |
|------|-------------|
| `-p, --path <dir>` | Path to local git repository |
| `-n, --name <name>` | Repository name |
| `-d, --description <d>` | Repository description (max 350 chars) |
| `--public` | Make repository public |
| `--private` | Make repository private |
| `-P, --push` | Push to remote after creation |
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
4. Copy and use the token

## Configuration

The tool stores your token in `~/.gh-repo-create.json`. You can also use the `GH_TOKEN` environment variable.

## License

MIT
