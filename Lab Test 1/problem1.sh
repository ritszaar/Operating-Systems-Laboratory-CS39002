echo -n "Enter x: "
read -r x
if [ $x -le 0 ]; then
    echo "invalid input"
else
    echo -n "Enter y: "
    read y
    if [ $y -le 0 ]; then
        echo "invalid input"
    else 
        echo -n "Enter m: "
        read m
        if [ $m -le 0 ]; then
            echo "invalid input"
        else
            echo $((x ** ((y * y) % m) % m))
        fi
    fi
fi