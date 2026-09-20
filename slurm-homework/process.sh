#!/bin/bash

src=$1
if [ -e "$src" ]; then
    IFS=$'\n' read -d '' -r -a values < $1
else
    echo "$src not such file!"
fi

sum=0
for(( i = 0 ; i<10 ; i++ )); do
    sum=$(bc -l <<< "${values[i]}+$sum" )
done
echo "Average execution time for correlation.c: $(bc -l <<< "scale=3;${sum}/10")" >> output_times.txt

sum=0
for(( i = 10 ; i<20 ; i++ )); do
    sum=$(bc -l <<< "${values[i]}+$sum" )
done
echo "Average execution time for doitgen.c: $(bc -l <<< "scale=3;${sum}/10")" >> output_times.txt

sum=0
for(( i = 20 ; i<30 ; i++ )); do
    sum=$(bc -l <<< "${values[i]}+$sum" )
done
echo "Average execution time for dynprog.c: $(bc -l <<< "scale=3;${sum}/10")" >> output_times.txt

sum=0
for(( i = 30 ; i<40 ; i++ )); do
    sum=$(bc -l <<< "${values[i]}+$sum" )
done
echo "Average execution time for floyd-warshall.c: $(bc -l <<< "scale=3;${sum}/10")" >> output_times.txt

sum=0
for(( i = 40 ; i<50 ; i++ )); do
    sum=$(bc -l <<< "${values[i]}+$sum" )
done
echo "Average execution time for jacobi-2d-imper.c: $(bc -l <<< "scale=3;${sum}/10")" >> output_times.txt
