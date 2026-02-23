#!/bin/bash

set -e

check_github_auth() {
    local token="${GITHUB_TOKEN:-}"
    if [ -n "$token" ]; then
        echo "Using GITHUB_TOKEN from environment"
        return 0
    fi
    
    if command -v gh &>/dev/null; then
        if gh auth status &>/dev/null; then
            echo "GitHub CLI authenticated"
            return 0
        fi
    fi
    
    if git ls-remote --heads https://github.com/test/test.git &>/dev/null 2>&1; then
        echo "Git authentication working"
        return 0
    fi
    
    echo "No GitHub authentication found. Options:"
    echo "  1) Set GITHUB_TOKEN environment variable"
    echo "  2) Run 'gh auth login' to authenticate with GitHub CLI"
    echo "  3) Run 'git config --global credential.helper store' and enter credentials once"
    return 1
}

configure_https_with_token() {
    local token="${1:-}"
    local repo_path="${2:-.}"
    
    if [ -z "$token" ]; then
        echo "Error: GitHub token required"
        echo "Usage: $0 --token <token> [repo_path]"
        return 1
    fi
    
    cd "$repo_path"
    local current_url=$(git remote get-url origin)
    local new_url
    if [[ "$current_url" == https://* ]]; then
        new_url="${current_url#https://}"
        new_url="https://x-access-token:${token}@${new_url}"
    elif [[ "$current_url" == git@* ]]; then
        local host="${current_url#git@}"
        host="${host%:*/}"
        new_url="https://x-access-token:${token}@${host}/${current_url##*:}"
    else
        echo "Unknown remote URL format: $current_url"
        return 1
    fi
    git remote set-url origin "$new_url"
    echo "Updated remote URL to use token authentication"
    echo "Testing connection..."
    git ls-remote --heads origin &>/dev/null && echo "Authentication working!" || echo "Authentication failed"
}

resolve_push_conflict() {
    local repo_path="${1:-.}"
    local remote="${2:-origin}"
    local branch="${3:-main}"
    local strategy="${4:-interactive}"
    
    cd "$repo_path"
    
    echo "=== Git Push Conflict Resolver ==="
    echo "Repository: $repo_path"
    echo "Remote: $remote"
    echo "Branch: $branch"
    echo ""
    
    if ! git remote get-url "$remote" &>/dev/null; then
        echo "Error: Remote '$remote' not found"
        return 1
    fi
    
    echo "Fetching from remote..."
    local fetch_output
    if fetch_output=$(git fetch "$remote" 2>&1); then
        echo "Fetch successful"
    else
        echo "Warning: Fetch failed - $fetch_output"
        echo ""
        echo "Possible causes:"
        echo "  - No authentication configured"
        echo "  - Network issues"
        echo "  - Repository doesn't exist"
        echo ""
        read -p "Try to push anyway? (y/n): " confirm
        if [ "$confirm" != "y" ]; then
            return 1
        fi
    fi
    
    local local_hash remote_hash
    local_hash=$(git rev-parse "$branch" 2>/dev/null)
    remote_hash=$(git rev-parse "$remote/$branch" 2>/dev/null)
    
    if [ -z "$remote_hash" ]; then
        echo "Remote branch not found. This is a fresh push."
        read -p "Push to create remote branch? (y/n): " confirm
        if [ "$confirm" = "y" ]; then
            git push -u "$remote" "$branch"
            return $?
        else
            return 1
        fi
    fi
    
    if [ "$local_hash" = "$remote_hash" ]; then
        echo "Local and remote are in sync. Pushing..."
        git push "$remote" "$branch"
        return $?
    fi
    
    echo "Local commit:  $local_hash"
    echo "Remote commit: $remote_hash"
    echo ""
    
    case "$strategy" in
        rebase|1)
            echo "Using rebase strategy..."
            git pull --rebase "$remote" "$branch"
            git push "$remote" "$branch"
            ;;
        merge|2)
            echo "Using merge strategy..."
            git pull "$remote" "$branch"
            git push "$remote" "$branch"
            ;;
        force|3)
            echo "Using force push (WARNING: overwrites remote)..."
            git push --force "$remote" "$branch"
            ;;
        inspect|4)
            echo "=== Local unique commits ==="
            git log "$remote/$branch".."$branch" --oneline || true
            echo ""
            echo "=== Remote unique commits ==="
            git log "$branch".."$remote/$branch" --oneline || true
            ;;
        interactive|*)
            resolve_push_conflict_interactive "$repo_path" "$remote" "$branch"
            ;;
    esac
}

resolve_push_conflict_interactive() {
    local repo_path="$1"
    local remote="$2"
    local branch="$3"
    
    echo "Divergence detected! Options:"
    echo "  1) pull-rebase  - Fetch and rebase on top of remote (recommended)"
    echo "  2) pull-merge   - Fetch and merge remote changes"
    echo "  3) force-push  - Force push (WARNING: overwrites remote)"
    echo "  4) inspect      - Show diff between branches"
    echo "  5) abort        - Cancel operation"
    echo ""
    read -p "Choose option (1-5): " choice
    
    case "$choice" in
        1)
            echo "Performing pull --rebase..."
            git pull --rebase "$remote" "$branch"
            echo "Pushing to remote..."
            git push "$remote" "$branch"
            ;;
        2)
            echo "Performing pull (merge)..."
            git pull "$remote" "$branch"
            echo "Pushing to remote..."
            git push "$remote" "$branch"
            ;;
        3)
            echo "WARNING: Force pushing will overwrite remote history!"
            read -p "Type 'force' to confirm: " confirm
            if [ "$confirm" = "force" ]; then
                git push --force "$remote" "$branch"
            else
                echo "Cancelled"
                return 1
            fi
            ;;
        4)
            echo "=== Local commits not in remote ==="
            git log "$remote/$branch".."$branch" --oneline
            echo ""
            echo "=== Remote commits not in local ==="
            git log "$branch".."$remote/$branch" --oneline
            ;;
        5)
            echo "Aborted"
            return 1
            ;;
        *)
            echo "Invalid option"
            return 1
            ;;
    esac
}

test_framework() {
    echo "=== Testing Git Push Resolver Framework ==="
    echo ""
    
    echo "Test 1: Check authentication"
    if check_github_auth; then
        echo "✓ Auth check passed"
    else
        echo "✗ Auth check failed (this is expected if not configured)"
    fi
    echo ""
    
    echo "Test 2: Show current repo status"
    cd ../conmdview
    echo "Remote: $(git remote get-url origin)"
    echo "Branch: $(git branch --show-current)"
    echo "Last commit: $(git log -1 --oneline)"
    echo ""
    
    echo "Test 3: Attempt to resolve push"
    resolve_push_conflict "../conmdview" "origin" "main" "inspect"
}

if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    case "${1:-}" in
        --token|-t)
            configure_https_with_token "$2" "${3:-.}"
            ;;
        --check-auth)
            check_github_auth
            ;;
        --test)
            test_framework
            ;;
        --auto)
            resolve_push_conflict "${2:-.}" "${3:-origin}" "${4:-main}" "rebase"
            ;;
        "")
            echo "Git Push Conflict Resolver"
            echo ""
            echo "Usage:"
            echo "  $0 <repo_path> [remote] [branch] [strategy]"
            echo "  $0 --token <github_token> [repo_path]"
            echo "  $0 --check-auth"
            echo "  $0 --test"
            echo "  $0 --auto <repo_path> [remote] [branch]"
            echo ""
            echo "Strategies: rebase (1), merge (2), force (3), inspect (4), interactive (default)"
            ;;
        *)
            resolve_push_conflict "$1" "${2:-origin}" "${3:-main}" "${4:-interactive}"
            ;;
    esac
fi
