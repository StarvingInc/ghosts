#include "game.hpp"
#include <iostream>
#include <vector>
#include <cstring>

enum Command {BOARD, WIN, LOSE, MOVE};

char* get_board(char id, char **red, char **blue)
{
	char *board = new char[36];
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
	return board;
}

inline char find(char a, char *tab) {
	char ret = -1;
	for(int i = 0; i < 4; ++i)
		if(tab[i] == a)
			ret = i;
	return ret;
}

bool validate_move(char id, char *move, char **red, char ** blue) {
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

void game(sf::TcpSocket *socket)
{
	char buf[32];
	std::size_t size;
	bool running = true;
	char act_player = 0;
	Command command;
/*
 * Sending ID of player to him
 */
	buf[0] = 0; 
	socket[0].send(buf, 1);
	buf[0] = 1;
	socket[1].send(buf, 1);
	for(int i = 0; i < 2; ++i) {
		socket[i].receive(buf, 1, size);
	}
/*
 * Geting initial position of ghosts from players
 */
	char *red[2], *blue[2];
	for(int i = 0; i < 2; ++i) {
		red[i] = new char[4];
		blue[i] = new char[4];
		socket[i].receive(buf, 8, size);
		memcpy(red[i], buf, 4);
		memcpy(blue[i], buf + 4, 4);
	}
/*
 * Sending board status and geting next move from player
 */
	while(running) {
		for(int i = 0; i < 2; ++i) {
			command = BOARD;
			socket[i].send(&command, sizeof(command));
			socket[i].receive(buf, 1, size);
		}
		command = MOVE;
		socket[act_player].send(&command, sizeof(command));
		socket[act_player].receive(buf, 3, size);
		validate_move(act_player, buf, red, blue);

		act_player = 1 - act_player;
	}


	std::cerr << "close connections and free memory  ";
	socket[0].disconnect();
	socket[1].disconnect();
	delete socket;
	std::cerr << "DONE" << std::endl;
}
