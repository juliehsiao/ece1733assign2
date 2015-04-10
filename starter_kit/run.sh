#!/bin/bash
end=$1


for ((i=$1; i<=$end; i++))
do
    rm *.dot
    #rm *.pdf
    ./assign2 ../test_nodes/node${i}.blif $2 $3
    for file in *.dot
    do
        funcName="${file%.*}"
        echo "making node${i}_${funcName}.pdf $file"
        dot -Tpdf -o node${i}_${funcName}.pdf $file
    done
done
echo "done!"

