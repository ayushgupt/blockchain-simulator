g++ blockChainSimulator.cpp -std=c++11 -O3 -o exec.out
./exec.out $2 $3 $4 $5
./makeGraph.sh
mkdir $1
mv *.svg $1
mv *.py $1
mv *.txt $1