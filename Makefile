all:
	g++ -O3 -Wall simple.cpp -o build/simple

clean:
	rm -f build/simple

remake:
	rm -f build/simple
	g++ -O3 -Wall simple.cpp -o build/simple

