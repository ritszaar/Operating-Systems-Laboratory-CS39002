#!/bin/bash
myarr=(`find $1 -type f -name "*.py"`)
for line in "${myarr[@]}"
do
echo $line
echo $line |xargs grep -nE "#" | grep -vE "\".*?#.*?\"" | grep -oE "^[0-9]*|#.*$"
echo $line |xargs pcregrep -nM "\"\"\"(?s).*?\"\"\"" | pcregrep -M --om-separator=$ "^[0-9]*|\"\"\"(?s).*?\"\"\""
echo $line |xargs pcregrep -nM "\'\'\'(?s).*?\'\'\'" | pcregrep -M --om-separator=$ "^[0-9]*|\'\'\'(?s).*?\'\'\'"
done
