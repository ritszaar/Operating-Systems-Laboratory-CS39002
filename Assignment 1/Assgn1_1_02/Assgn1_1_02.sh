#!/bin/sh
ans=1
for y in $(rev $1 | head -n -1)
do
	x=$ans
	ty=$y
	while [ $y -gt 0 ]
	do
		temp=$(($x%$y))
		x=$y
		y=$temp
	done
	ans=$(($ans/$x))
	ans=$(($ans*$ty))
done
echo $ans
