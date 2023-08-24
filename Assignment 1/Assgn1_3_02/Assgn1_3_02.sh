#! /usr/bin/bash

args=( "$@" )

for file_name in `ls $1 | sed 's/.jsonl//g'`; do
    echo ${args[@]:2} | sed 's/ /,/g' > $2/$file_name.csv
    formatter="` echo "[(.${args[@]:2}|tostring)]" | sed 's/ /|tostring),(./g' ` | @csv"
    while read -r line; do
        jq -r "$formatter" <<< "$line" >> $2/$file_name.csv
    done < $1/$file_name.jsonl
done
