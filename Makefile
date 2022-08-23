#Butilca Rares + Anton Alexandra

build: hlt.hpp MyBot.cpp networking.hpp
	g++ -std=c++11 MyBot.cpp -o MyBot

run:
	./MyBot

clean:
	rm -f MyBot



