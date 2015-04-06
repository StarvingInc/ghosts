#include <iostream>
#include <thread>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <SFML/Network.hpp>
#include "game.hpp"

void game(int *socket)
{
	char buf[36];
	std::size_t size;
	bool running = true;
	char act_player = 0;
	Command command, status = OK;
	char taken_red[2] = {0}, taken_blue[2] = {0};

/*
 * Sending ID of player to him
 */
	buf[0] = 0; 
	if(send(socket[0], buf, 1, 0) != 1) {
		std::cerr << "ERROR while sending id to client 0" << std::endl;
		return;
	}
	buf[0] = 1;
	if(send(socket[1], buf, 1, 0) != 1) {
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
		if(recv(socket[i], buf, 8, 0) != 8) {
			std::cerr << "ERROR while receiving initial ghost positions from player " << i << std::endl;
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
			if(send(socket[i], &command, sizeof command, 0) != sizeof command) {
				std::cerr << "ERROR while sending command BOARD to player  " << i << std::endl;
				return;
			}
			if(recv(socket[i], buf, 1, 0) != 1) {
				std::cerr << "ERROR while receiving answer from player  " << i << std::endl;
				return;
			}
			get_board(i, buf, red, blue, taken_red, taken_blue);
			if(send(socket[i], buf, 38, 0) != 38) {
				std::cerr << "ERROR while sending board status to player  " << i << std::endl;
				return;
			}
			if(recv(socket[i], buf, 1, 0) != 1) {
				std::cerr << "ERROR while receiving board confirmation from player  " << i << std::endl;
				return;
			}
		}
		if(status == WIN) {
			command = LOSE;
			if(send(socket[act_player], &command, sizeof command, 0) != sizeof command) {
				std::cerr << "ERROR while sending LOSE command to player  " << static_cast<int>(act_player) << std::endl;
				return;
			}
			command = WIN;
			if(send(socket[1 - act_player], &command, sizeof command, 0) != sizeof command) {
				std::cerr << "ERROR while sending WIN command to player  " << static_cast<int>(1 - act_player) << std::endl;
				return;
			}
			break;
		}
		else if(status == LOSE) {
			command = WIN;
			if(send(socket[act_player], &command, sizeof command, 0) != sizeof command) {
				std::cerr << "ERROR while sending WIN command to player  " << static_cast<int>(act_player) << std::endl;
				return;
			}
			command = LOSE;
			if(send(socket[1 - act_player], &command, sizeof command, 0) != sizeof command) {
				std::cerr << "ERROR while sending LOSE command to player  " << static_cast<int>(1 - act_player) << std::endl;
				return;
			}
			break;
		}
		else if(status == OK) {
			command = MOVE;
			if(send(socket[act_player], &command, sizeof command, 0) != sizeof command) {
				std::cerr << "ERROR while sending MOVE command to player  " << static_cast<int>(act_player) << std::endl;
				return;
			}
			if(recv(socket[act_player], buf, 3, 0) != 3) {
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
	for(int i = 0; i < 2; ++i) {
		shutdown(socket[i], 2);
		delete red[i];
		delete blue[i];
	}
	delete socket;
	std::cerr << "DONE" << std::endl;
}

int main(int argc, char **argv)
{
	bool running = true;
	
	int listener, status;
	sockaddr_storage their_addr;
	socklen_t addr_size = sizeof their_addr;
	addrinfo hints, *res;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	char *port;
	if(argc > 1)
		port = argv[1];
	else
		port = "32332";

	if((status = getaddrinfo(NULL, port, &hints, &res)) != 0) {
		std::cerr << gai_strerror(status) << std::endl;
		exit(1);
	}
	listener = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if(listener == -1) {
		std::cerr << strerror(errno) << std::endl;
		exit(1);
	}
	bind(listener, res->ai_addr, res->ai_addrlen);
	listen(listener, 10);


	int *clients;
	int ctr = 0;
	while(running) {
		if(ctr == 0)
			clients = new int[2];
		clients[ctr] = accept(listener, (sockaddr *)&their_addr, &addr_size);
		if(ctr == 1) {
			std::cerr << "starting new game" << std::endl;
			game(clients);
			//std::thread game_thread(game, clients);
			//game_thread.detach();
		}
		ctr = 1 - ctr;
	}
}
