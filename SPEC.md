# GitHub Repo Creator - Specification

## Project Overview

- **Project name**: gh-repo-create
- **Type**: Modern Node.js CLI tool with REPL interface
- **Core functionality**: Create GitHub repositories from existing local git projects with interactive prompts
- **Target users**: Developers who want a streamlined workflow to publish local git projects to GitHub

## Technical Stack

- **Language**: TypeScript
- **Runtime**: Node.js
- **Key dependencies**:
  - `octokit` - GitHub REST API
  - `inquirer` - Interactive prompts
  - `chalk` - Terminal styling
  - `commander` - CLI argument parsing
  - `simple-git` - Git operations

## UI/UX Specification

### Layout Structure

- **REPL Mode**: Interactive shell with prompt `gh-repo> `
- **CLI Mode**: Direct command execution with flags
- **Header**: Tool name and version on startup
- **Footer**: Helpful usage hints

### Visual Design

**Color Palette**:
- Primary: `#58a6ff` (GitHub blue)
- Success: `#3fb950` (green)
- Error: `#f85149` (red)
- Warning: `#d29922` (yellow)
- Text: `#c9d1d9` (light gray)
- Muted: `#8b949e` (gray)

**Typography**:
- Font: System monospace
- Headings: Bold, colored
- Body: Regular weight

**Spacing**:
- Standard padding: 1rem
- Between sections: 2rem

### Components

1. **Welcome Banner**
   - Tool name with ASCII art
   - Version info

2. **Authentication Prompt**
   - Check for GitHub token
   - Prompt for token if missing

3. **Repo Creation Flow**
   - Step 1: Select local git folder
   - Step 2: Enter repo name (validated)
   - Step 3: Enter description (max 350 chars with counter)
   - Step 4: Choose visibility (public/private)
   - Step 5: Confirm and create
   - Step 6: Push to remote

4. **Status Messages**
   - Loading spinners for async operations
   - Success/error notifications

## Functionality Specification

### Core Features

1. **GitHub Authentication**
   - Uses GitHub Personal Access Token
   - Token stored in `~/.gh-repo-create.json` or environment variable `GH_TOKEN`
   - Validates token on startup

2. **Local Git Project Detection**
   - Validates selected folder is a git repository
   - Reads current remote if exists
   - Gets current branch name

3. **Repository Creation**
   - Creates repo via GitHub API
   - Options: name, description, public/private
   - Auto-generates README if requested

4. **Git Push**
   - Adds GitHub remote if not present
   - Pushes all branches
   - Sets upstream

### User Interactions

1. **REPL Commands**:
   - `create` or `c` - Start creation flow
   - `auth` - Manage authentication
   - `help` - Show help
   - `exit` - Exit REPL

2. **CLI Flags**:
   - `--path <dir>` - Local git directory
   - `--name <name>` - Repo name
   - `--description <desc>` - Repo description
   - `--public` - Public repo
   - `--private` - Private repo
   - `--push` - Auto-push after creation

### Data Handling

- Token stored in JSON config file
- No sensitive data logged
- API calls use Octokit with proper error handling

### Edge Cases

- Invalid git directory → Show error, suggest valid path
- Token expired → Prompt for re-authentication
- Repo name already exists → Offer to use different name
- No internet → Show connectivity error
- Description too long → Enforce 350 char limit with counter

## Acceptance Criteria

1. ✓ Tool starts with welcome banner
2. ✓ Authenticates with GitHub token (prompts if missing)
3. ✓ Validates local folder is a git repo
4. ✓ Prompts for repo name (validates format)
5. ✓ Prompts for description (enforces 350 char limit)
6. ✓ Prompts for public/private with clear options
7. ✓ Creates repo via GitHub API
8. ✓ Adds remote and pushes to GitHub
9. ✓ REPL mode works with all commands
10. ✓ CLI mode works with flags
11. ✓ Error handling for all edge cases
