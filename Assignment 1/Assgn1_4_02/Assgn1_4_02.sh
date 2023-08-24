#!/bin/bash
while read line
do
temp=`echo "$line" | grep -wc $2`
[ $temp -gt 0 ] && echo "$line" | sed "s/[A-Z]/\L&/g" | sed -E "s/([a-z])([^a-z]*)(.)/\L\1\2\U\3/g" || echo "$line"
done < $1
