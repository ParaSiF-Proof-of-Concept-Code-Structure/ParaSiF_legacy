#!/bin/bash

counter=1

# Rename files matching the pattern
for file in 0.*; do
    if [[ $file =~ 0\.([0-9]+) ]]; then
        new_name=$counter
        counter=$((counter + 1))
        mv "$file" "$new_name"
    fi
done
