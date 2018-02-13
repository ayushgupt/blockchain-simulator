all:
	g++ blockChainSimulator.cpp -std=c++11
	./a.out
clean:
	rm -f *.png
	rm -f *.py
	rm -f a.out
