#!/bin/bash
end=$1


for ((i=$1; i<=$end; i++))
do
    rm *.dot
    rm *.ps
    ./assign2 ../test_nodes/node${i}.blif $2 $3
    j=0
    for file in *.dot
    do
        echo "making node${i}_${j}.ps $file"
        funcName="${file%.*}"
        dot -Tps -o node${i}_${funcName}.ps $file
        j=$((j+1))
    done
done
echo "done!"

