#!/bin/bash

# Get a list of files in the current directory
files=($(ls -1 | grep -E '^[0-9]+\.*[0-9]*$' | sort -n))

# Initialize a counter
count=0

# Rename files in ascending order
for file in "${files[@]}"; do
    new_name="$count"
    # Pad with zeros if count is less than 10
    if [ "$count" -lt 10 ]; then
        new_name="$new_name"
    fi
    mv "$file" "$new_name"
    count=$((count+1))
done
