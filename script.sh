#!/bin/bash

cd /home/script

for file in *.c; do
    if [ -f "$file" ]; then
        basename="${file%.c}"
        gcc -o "${basename}.o" "$file"
        echo "Compiled: $file -> ${basename}.o"
    fi
done