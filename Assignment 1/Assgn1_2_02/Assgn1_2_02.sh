#!/bin/bash
>$3
while read uname
do
	while read fruit
	do
		[[ ! "${uname,,}" =~ ^[a-z][a-z0-9]{4,19}$ || ! "${uname,,}" =~ ^.*[0-9].*$ || "${uname,,}" =~ ^.*"${fruit,,}".*$ ]] && echo "NO">>$3 && continue 2
	done <$2
	echo "YES">>$3
done <$1