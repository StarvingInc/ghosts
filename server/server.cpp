#include <iostream>
#include <thread>
#include <cstdlib>
#include "game.hpp"

int main(int argc, char **argv)
{
	bool running = true;
	
	int port = 32332;
	if(argc > 1)
		port = atoi(argv[1]);

	sf::TcpListener listener;
	listener.listen(port);
	sf::TcpSocket *clients;
	int ctr = 0;
	while(running) {
		if(ctr == 0)
			clients = new sf::TcpSocket[2];
		listener.accept(clients[ctr]);
		if(ctr == 1) {
			std::cerr << "starting new game" << std::endl;
			std::thread game_thread(game, clients);
		}
		ctr = 1 - ctr;
	}
}
