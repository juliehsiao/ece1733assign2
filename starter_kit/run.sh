#!/bin/bash
end=15

make
for ((i=1; i<=$end; i++))
do
    rm BDD*.dot
    ./assign2 ../test_nodes/node${i}.blif
    j=0
    for file in ./*.dot
    do
        echo "making node${i}_${j}.pdf $file"
        dot -Tpdf -o node${i}_${j}.pdf $file
        j=$((j+1))
    done
done
echo "done!"

