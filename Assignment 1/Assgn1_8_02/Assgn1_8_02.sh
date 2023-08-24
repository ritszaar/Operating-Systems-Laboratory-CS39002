#! /usr/bin/bash

args=( "$@" )

for i in ${!args[@]}; do
    if grep -q '[0-9][0-9]-[0-9][0-9]-[0-9][0-9]' <<< ${args[$i]}; then
        entry="${args[$i]},${args[$((i + 1))]},${args[$((i + 2))]},${args[$((i + 3))]}"
        echo $entry >> "./main.csv"
        echo Inserts $entry into the csv
    fi
done

IFS=','

for i in ${!args[@]}; do
    if [ "${args[$i]}" = "-c" ]; then
        amount_spent=0
        category=${args[$((i + 1))]}
        while read -r line; do
            fields=( $line )
            if [ "${fields[1]}" = "$category" ]; then
                amount_spent=$(($amount_spent + ${fields[2]}))
            fi
        done < "./main.csv"
        echo Total amount spent on $category = $amount_spent
    fi
done

for i in ${!args[@]}; do
    if [ "${args[$i]}" = "-n" ]; then
        amount_spent=0
        name=${args[$((i + 1))]}
        while read -r line; do
            fields=( $line )
            if [ "${fields[3]}" = "$name" ]; then
                amount_spent=$(($amount_spent + ${fields[2]}))
            fi
        done < "./main.csv"
        echo Total amount spent by $name = $amount_spent
    fi
done

for i in ${!args[@]}; do
    if [ "${args[$i]}" = "-s" ]; then
        column_name=${args[$((i + 1))]}
        if [ "$column_name" = "Date" ]; then
            sort -t, -k1,1 -o main.csv{,}
            touch temp.csv
            while read -r entry; do
                fields=( $entry )
                IFS='-'
                date_parts=( ${fields[0]} )
                fields[0]="${date_parts[2]}-${date_parts[1]}-${date_parts[0]}"
                IFS=','
                echo ${fields[@]} | sed 's/ /,/g' >> temp.csv
            done < "./main.csv"

            sort -t, -k1,1 -n -o temp.csv{,}
            rm main.csv
            while read -r entry; do
                fields=( $entry )
                IFS='-'
                date_parts=( ${fields[0]} )
                fields[0]="${date_parts[2]}-${date_parts[1]}-${date_parts[0]}"
                IFS=','
                echo ${fields[@]} | sed 's/ /,/g' >> main.csv
            done < "./temp.csv"
            rm temp.csv
            echo "Sort the csv according to 'date' column"
        elif [ "$column_name" = "Category" ]; then
            sort -t, -k2,2 -o main.csv{,}
            echo "Sort the csv according to 'category' column"
        elif [ "$column_name" = "Amount" ]; then
            sort -t, -k3,3 -n -o main.csv{,}
            echo "Sort the csv according to 'amount' column"
        else
            sort -t, -k4,4 -o main.csv{,}
            echo "Sort the csv according to 'name' column"
        fi
    fi
done

IFS=' '

for i in ${!args[@]}; do
    if [ "${args[$i]}" = "-h" ]; then
        inside="yes"
        while [ "$inside" = "yes" ]; do
            echo
            echo -n "Enter the name of the operation [insertion, category_expense, name_expense, sort]: "
            read instr
            if [ "$instr" = "insertion" ]; then
                echo ''
                echo "Insertion takes place before any other operation in one single call. To insert, command must be given in the following manner: "
                echo
                echo "./Assgn_1_8_02.sh Date (dd-mm-yy) | Category | Amount | Name [..Records]"
                echo
                echo "The above command inserts records in main.csv."
                echo
                echo "Example: ./Assgn1_8_02.sh 01-01-20 gaming 1000 amanda"
            elif [ "$instr" = "category_expense" ]; then
                echo ''
                echo "Categorical expenses can be easily determined. To find categorical expense, command must be given in the following manner: "
                echo
                echo "./Assgn_1_8_02.sh -c 'Category'"
                echo
                echo "The above prints the total expenses incurred in 'Category'."
                echo
                echo "Example: ./Assgn1_8_02.sh -c Date"
            elif [ "$instr" = "name_expense" ]; then
                echo ''
                echo "Individual expenses can be easily found out. To find individual expense, command must be given in the following manner: "
                echo
                echo "./Assgn_1_8_02.sh -n 'Name'"
                echo
                echo "The above prints the total expenses of 'Name'."
                echo
                echo "Example: ./Assgn1_8_02.sh -n Ciri"
            elif [ "$instr" = "sort" ]; then
                echo ''
                echo "The data can be easily sorted. To sort by a given column, command must be given in the following manner: "
                echo
                echo "./Assgn_1_8_02.sh -s 'Column'"
                echo
                echo "The above commands sorts the entries in main.csv with respect to 'Column' field."
                echo
                echo "Example: ./Assgn1_8_02.sh -s Date"
            else    
                echo
                echo "Please select a valid operation."
            fi

            echo
            echo -n "Do you need further assisstance [yes|no]: "
            read inside
        done
    fi
done
