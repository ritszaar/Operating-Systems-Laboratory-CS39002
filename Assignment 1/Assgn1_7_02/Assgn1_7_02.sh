#!/bin/bash
mkdir -p "$2"
for file_name in `ls $1`; do
    while read -r name; do
        echo $name >> "$2/${name:0:1}.txt"  
    done < "$1/$file_name"
done
for file_name in `ls $2`; do
    sorted_names=`sort "$2/$file_name"`
    echo "$sorted_names" > "$2/$file_name"  y
done
