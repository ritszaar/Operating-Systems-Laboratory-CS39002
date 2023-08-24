#!/bin/bash
for ((i=0; i<=1000000; i++)); do
is_prime[$i]=1
done
for ((i=2; i*i<=1000000; i++)); do
if [ ${is_prime[$i]} -eq 1 ]; then
for ((j=i*i; j<=1000000; j+=i)); do
is_prime[$j]=0
done
fi
done
echo -n ""
while read -r num; do
for ((i=2; i<=num; i++)); do
if [[ ${is_prime[$i]} -eq 1 && $((num%i)) -eq 0 ]]; then
echo -n "$i " 
fi
done
echo ""
done < "./input.txt" > "./output.txt"
