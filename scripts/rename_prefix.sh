#!/bin/bash
# This script renames files by removing the "@3x-" prefix.

shopt -s nullglob  # If no files match, do nothing.
for file in @3x-*; do
    newname="${file#@3x-}"
    echo "Renaming '$file' to '$newname'"
    mv "$file" "$newname"
done
