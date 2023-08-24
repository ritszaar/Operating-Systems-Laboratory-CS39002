#!/bin/bash
cut -d ' ' -f 2 $1 | uniq -c | awk ' { t = $1; $1 = $2; $2 = t; print; } ' | sort | sort -k2,2nr -s
echo ""
cut -d ' ' -f 1 $1 | sort | uniq -d
echo $(($(cat $1 | wc -l)-$(cut -d ' ' -f 1 $1 | sort | uniq -D | wc -l)))
