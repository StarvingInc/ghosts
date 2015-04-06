#include <iostream>
#include <thread>
#include <cstdlib>
#include <cstring>
#include <SFML/Network.hpp>
#include "game.hpp"

void game(sf::TcpSocket *socket)
{
	char buf[38];
	std::size_t size;
	bool running = true;
	char act_player = 0;
	Command command, status = OK;
	char taken_red[2] = {0}, taken_blue[2] = {0};

/*
 * Sending ID of player to him
 */
	buf[0] = 0; 
	if(socket[0].send(buf, 1) != sf::Socket::Done) {
		std::cerr << "ERROR while sending id to client 0" << std::endl;
		return;
	}
	buf[0] = 1;
	if(socket[1].send(buf, 1) != sf::Socket::Done) {
		std::cerr << "ERROR while sending id to client 1" << std::endl;
		return;
	}
/*
 * Geting initial position of ghosts from players
 */
	char *red[2], *blue[2];
	for(int i = 0; i < 2; ++i) {
		red[i] = new char[4];
		blue[i] = new char[4];
		if(socket[i].receive(buf, 8, size) != sf::Socket::Done) {
			std::cerr << "ERROR while receiving initial ghost positions from player " << i << std::endl;
			return;
		}
		if(size != 8) {
			std::cerr << "ERROR: wrong amount of ytes received from player  " << i << std::endl;
			return;
		}
		memcpy(red[i], buf, 4);
		memcpy(blue[i], buf + 4, 4);
	}
	//validate recived data
	if(!validate_initial_data(red, blue)) {
		std::cerr << "ERROR initial data are wrong (cheating or client bug)" << std::endl;
		return;
	}
/*
 * Sending board status and geting next move from player
 * board format: 36 chars, each contaion info about one field
 * ( 0-nothing, 1- red ghost, 2-blue ghost, 3-enemie ghost)
 * move respond format:
 * move[0] : 1 when moving ghost is red, 2 when moving ghost is blue
 * move[1] : old position of ghost
 * move[2] : new position of ghost
 */
	while(running) {
		for(int i = 0; i < 2; ++i) {
			command = BOARD;
			if(socket[i].send(&command, sizeof(command)) != sf::Socket::Done) {
				std::cerr << "ERROR while sending command BOARD to player  " << i << std::endl;
				return;
			}
			if(socket[i].receive(buf, 1, size) != sf::Socket::Done) {
				std::cerr << "ERROR while receiving answer from player  " << i << std::endl;
				return;
			}
			get_board(i, buf, red, blue, taken_red, taken_blue);
			if(socket[i].send(buf, 36) != sf::Socket::Done) {
				std::cerr << "ERROR while sending board status to player  " << i << std::endl;
				return;
			}
			if(socket[i].receive(buf, 1, size) != sf::Socket::Done) {
				std::cerr << "ERROR while receiving board confirmation from player  " << i << std::endl;
				return;
			}
		}
		if(status == WIN) {
			command = LOSE;
			if(socket[act_player].send(&command, sizeof(command)) != sf::Socket::Done) {
				std::cerr << "ERROR while sending LOSE command to player  " << static_cast<int>(act_player) << std::endl;
				return;
			}
			command = WIN;
			if(socket[1 - act_player].send(&command, sizeof(command)) != sf::Socket::Done) {
				std::cerr << "ERROR while sending WIN command to player  " << static_cast<int>(1 - act_player) << std::endl;
				return;
			}
			break;
		}
		else if(status == LOSE) {
			command = WIN;
			if(socket[act_player].send(&command, sizeof(command)) != sf::Socket::Done) {
				std::cerr << "ERROR while sending WIN command to player  " << static_cast<int>(act_player) << std::endl;
				return;
			}
			command = LOSE;
			if(socket[1 - act_player].send(&command, sizeof(command)) != sf::Socket::Done) {
				std::cerr << "ERROR while sending LOSE command to player  " << static_cast<int>(1 - act_player) << std::endl;
				return;
			}
			break;
		}
		else if(status == OK) {
			command = MOVE;
			if(socket[act_player].send(&command, sizeof(command)) != sf::Socket::Done) {
				std::cerr << "ERROR while sending MOVE command to player  " << static_cast<int>(act_player) << std::endl;
				return;
			}
			if(socket[act_player].receive(buf, 3, size) != sf::Socket::Done) {
				std::cerr << "ERROR while reciving move description from player " << static_cast<int>(act_player) << std::endl;
				return;
			}
			//validate if player cheating via modification of client (or bug in client)
			if(!validate_move(act_player, buf, red, blue)) {
				std::cerr << "wrong move" << std::endl;
				return;
			}
			status = make_move(act_player, buf, red, blue, taken_red, taken_blue);
		}
		act_player = 1 - act_player;
	}

	std::cerr << "close connections and free memory  ";
	socket[0].disconnect();
	socket[1].disconnect();
	delete socket;
	for(int i = 0; i < 2; ++i) {
		delete red[i];
		delete blue[i];
	}
	std::cerr << "DONE" << std::endl;
}

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
			game_thread.detach();
		}
		ctr = 1 - ctr;
	}
}
