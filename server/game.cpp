#include "game.hpp"
#include <iostream>
#include <vector>
#include <cstring>

void get_board(char id, char *board, char **red, char **blue)
{
	memset(board, 0, 36);
	for(int i = 0; i < 4; ++i) {
		if(red[id][i] != -1)
			board[red[id][i]] = 1;
		if(blue[id][i] != -1)
			board[blue[id][i]] = 2;
		if(red[1 - id][i] != -1)
			board[red[1 - id][i]] = 3;
		if(blue[1 - id][i] != -1)
			board[blue[1 - id][i]] = 3;
	}
}

inline char find(char a, char *tab)
{
	char ret = -1;
	for(int i = 0; i < 4; ++i)
		if(tab[i] == a)
			ret = i;
	return ret;
}

bool validate_move(char id, char *move, char **red, char ** blue)
{
	if(move[0] != 1 && move[0] != 2)
		return false;
	if(move[1] < 0 || move[2] < 0)
		return false;
	if(move[0] == 1)
		if(find(move[1], red[id]) == -1)
			return false;
	if(move[0] == 2)
		if(find(move[1], blue[id]) == -1)
			return false;
	if(find(move[2], red[id]) != -1 || find(move[2], blue[id]) != -1)
		return false;
	if(move[1] % 6 != 0 && move[2] == (move[1] - 1))
		return true;
	if(move[1] % 6 != 5 && move[2] == (move[1] + 1))
		return true;
	if(move[1] / 6 != 0 && move[2] == (move[1] - 6))
		return true;
	if(move[1] / 6 != 5 && move[2] == (move[1] + 6))
		return true;
	return false;
}

Command make_move(char id, char *move, char **red, char **blue, char *taken_red, char *taken_blue)
{
	if(move[0] == 1)
		red[id][find(move[1], red[id])] = move[2];
	if(move[0] == 2)
		blue[id][find(move[1], blue[id])] = move[2];
	char position = find(move[2], red[1 - id]);
	if(position != -1){
		red[1 - id][position] = -1;
		taken_red[1 - id]++;
		if(taken_red[1 - id] == 4)
			return LOSE;
	}
	position = find(move[2], blue[1 - id]);
	if(position != -1) {
		blue[1 - id][position] = -1;
		taken_blue[1 - id]++;
		if(taken_blue[1 - id] == 4)
			return WIN;
	}
	if(id == 0 && move[0] == 2 && (move[2] == 30 || move[2] == 35))
		return WIN;
	if(id == 1 && move[0] == 2 && (move[2] == 0 || move[2] == 5))
		return WIN;
	return OK;
}

bool validate_initial_data(char **red, char **blue)
{
	for(int i = 1; i <= 4; ++i) {
		if(find(i, red[0]) == -1 && find(i, blue[0]) == -1)
			return false;
	}
	for(int i = 7; i <= 10; ++i) {
		if(find(i, red[0]) == -1 && find(i, blue[0]) == -1)
			return false;
	}
	for(int i = 34; i >= 31; --i) {
		if(find(i, red[1]) == -1 && find(i, blue[1]) == -1)
			return false;
	}
	for(int i = 28; i >= 25; --i) {
		if(find(i, red[1]) == -1 && find(i, blue[1]) == -1)
			return false;
	}
	return true;
}

void game(sf::TcpSocket *socket)
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
	if(socket[0].send(buf, 1) != sf::Socket::Done) {
		std::cerr << "ERROR while sending id to client 0" << std::endl;
		exit(1);
	}
	buf[0] = 1;
	if(socket[1].send(buf, 1) != sf::Socket::Done) {
		std::cerr << "ERROR while sending id to client 1" << std::endl;
		exit(1);
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
			exit(1);
		}
		if(size != 8) {
			std::cerr << "ERROR: wrong amount of ytes received from player  " << i << std::endl;
			exit(1);
		}
		memcpy(red[i], buf, 4);
		memcpy(blue[i], buf + 4, 4);
	}
	//validate recived data
	if(!validate_initial_data(red, blue)) {
		std::cerr << "ERROR initial data are wrong (chating or client bug)" << std::endl;
		exit(1);
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
				exit(1);
			}
			if(socket[i].receive(buf, 1, size) != sf::Socket::Done) {
				std::cerr << "ERROR while receiving answer from player  " << i << std::endl;
				exit(1);
			}
			get_board(i, buf, red, blue);
			if(socket[i].send(buf, 36) != sf::Socket::Done) {
				std::cerr << "ERROR while sending board status to player  " << i << std::endl;
				exit(1);
			}
			if(socket[i].receive(buf, 1, size) != sf::Socket::Done) {
				std::cerr << "ERROR while receiving board confirmation from player  " << i << std::endl;
				exit(1);
			}
		}
		if(status == WIN) {
			command = LOSE;
			if(socket[act_player].send(&command, sizeof(command)) != sf::Socket::Done) {
				std::cerr << "ERROR while sending LOSE command to player  " << static_cast<int>(act_player) << std::endl;
				exit(1);
			}
			command = WIN;
			if(socket[1 - act_player].send(&command, sizeof(command)) != sf::Socket::Done) {
				std::cerr << "ERROR while sending WIN command to player  " << static_cast<int>(1 - act_player) << std::endl;
				exit(1);
			}
			break;
		}
		else if(status == LOSE) {
			command = WIN;
			if(socket[act_player].send(&command, sizeof(command)) != sf::Socket::Done) {
				std::cerr << "ERROR while sending WIN command to player  " << static_cast<int>(act_player) << std::endl;
				exit(1);
			}
			command = LOSE;
			if(socket[1 - act_player].send(&command, sizeof(command)) != sf::Socket::Done) {
				std::cerr << "ERROR while sending LOSE command to player  " << static_cast<int>(1 - act_player) << std::endl;
				exit(1);
			}
			break;
		}
		else if(status == OK) {
			command = MOVE;
			if(socket[act_player].send(&command, sizeof(command)) != sf::Socket::Done) {
				std::cerr << "ERROR while sending MOVE command to player  " << static_cast<int>(act_player) << std::endl;
				exit(1);
			}
			if(socket[act_player].receive(buf, 3, size) != sf::Socket::Done) {
				std::cerr << "ERROR while reciving move description from player " << static_cast<int>(act_player) << std::endl;
				exit(0);
			}
			//validate if player cheating via modification of client (or bug in client)
			if(!validate_move(act_player, buf, red, blue)) {
				std::cerr << "wrong move" << std::endl;
				exit(1);
			}
			status = make_move(act_player, buf, red, blue, taken_red, taken_blue);
		}
		act_player = 1 - act_player;
	}

	std::cerr << "close connections and free memory  ";
	socket[0].disconnect();
	socket[1].disconnect();
	delete socket;
	std::cerr << "DONE" << std::endl;
}
