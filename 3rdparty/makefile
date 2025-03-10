.PHONY: json.a

json.a:
	g++ -c json-cpp/json.cpp -o json.o -std=c++23 -O2
	ar rcs libjson.a json.o
	rm -f json.o
