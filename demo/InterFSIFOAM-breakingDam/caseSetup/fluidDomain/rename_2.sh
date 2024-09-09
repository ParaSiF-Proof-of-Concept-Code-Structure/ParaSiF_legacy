#!/bin/bash

# Start loop from 1 to 200
for ((i=1; i<=460; i++))
do
    # Determine the appropriate number of digits after the decimal point
    if (( i < 10 )); then
        digits=1
    elif (( i < 100 )); then
        digits=2
    else
        digits=3
    fi

    # Old folder name
    old_name="0.$(printf "%0${digits}f" $i)"

    # New folder name
    new_name="$i"

    # Rename the folder
    mv "$old_name" "$new_name"
done

