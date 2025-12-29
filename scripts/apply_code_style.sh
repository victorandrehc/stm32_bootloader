#!/bin/bash

set -euo pipefail

extensions=("cpp" "c" "hpp" "h" "cpp.in" "c.in" "h.in" "hpp.in")
ignored_dirs=("Drivers")

format_files() {
    local dir="$1"

    # Build prune expression
    prune_expr=()
    for ignore in "${ignored_dirs[@]}"; do
        prune_expr+=( -path "$dir/$ignore" -o )
    done
    unset 'prune_expr[-1]'  # remove last -o

    for ext in "${extensions[@]}"; do
        find "$dir" \
            \( "${prune_expr[@]}" \) -prune \
            -o -type f -name "*.$ext" \
            -exec clang-format -i --style=file {} \;
    done
}

target_dir="${1:-.}"
format_files "$target_dir"
