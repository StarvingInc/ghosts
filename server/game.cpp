#include "game.hpp"
#include <iostream>

enum Command {BOARD, WIN, LOSE, MOVE};

void game(sf::TcpSocket *socket)
{
	/*
	 * Sending ID of player to him
	 */
	{
		std::size_t size;
		char id[1];
		id[0] = 0; 
		socket[0].send(id, 1);
		id[0] = 1;
		socket[1].send(id, 1);
		for(int i = 0; i < 2; ++i) {
			socket[i].receive(id, 1, size);
		}
	}

	std::cerr << "close connections and free memory  ";
	socket[0].disconnect();
	socket[1].disconnect();
	delete socket;
	std::cerr << "DONE" << std::endl;
}
